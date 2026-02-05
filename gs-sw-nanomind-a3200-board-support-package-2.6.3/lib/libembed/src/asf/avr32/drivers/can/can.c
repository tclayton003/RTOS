/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   AVR32UC3C CAN driver
*/

#include <gs/embed/drivers/can/can.h>

#include <gpio.h>
#include <status_codes.h>
#include <canif.h>

#include <gs/embed/freertos.h>
#include <gs/asf/avr32/asf.h>
#include <gs/util/error.h>
#include <gs/util/stdio.h>
#include <gs/util/string.h>
#include <gs/util/log.h>
#include <gs/util/check.h>

// change default log group
#define LOG_DEFAULT gs_can_log

/*
 * The processor has 16 mailboxes (MObs) that we split into two partitions: 4
 * for transmissions and 12 for reception. The reception mailboxes are further
 * divided in two FIFO partitions; 6 for "normal" reception and 6 for frame
 * "overflow".
 *
 *                   +----------------+
 *                 ^ |                | 15
 *                 | | Overflow MObs  |
 *                 + |                | 10
 *                   +----------------+
 *                 ^ |                | 9
 *                 | | RX MObs        |
 *                 + |                | 4
 *                   +----------------+
 *  Lower priority ^ |                | 3
 *                 | | TX MObs        |
 * Higher priority + |                | 0
 *                   +----------------+
 *
 * When multiple mailboxes are enabled for transmission, the first mailbox to
 * be sent is the one with the lowest CAN identifier (highest CAN frame
 * priority). Since CFP does not split CSP packets in frames with increasing
 * frame identifiers, loading of frames from one packet into multiple mailboxes
 * could lead to frame reordering and dropped packets. For this reason,
 * can_send blocks until completion of the mailbox. Note that transmissions on
 * _different_ CSP connections will use different TX mailboxes.
 *
 * All RX mailboxes are loaded with the same match identifier. When multiple
 * mailboxes are enabled for frame reception, the lowest numbered mailbox that
 * matches receives the frame. This makes it difficult to guarantee frame
 * order, which is required by CFP csp_can_rx_frame.
 *
 * We handle this by dividing the RX mailboxes in two (similar to the Linux
 * SocketCAN rx-fifo): the lowest 6 (with highest priority) are used first, but
 * are _not_ reenabled when a frame is received. At some point, these 6 MObs
 * are all used and we start using the overflow mailboxes. When this happens,
 * we reenable all regular RX mailboxes and mark that the RX IRQ _must_ scan
 * the overflow MObs first on the next interrupt.
 */

/* MOB segmentation */
#define CAN_TX_MBOX     4
#define CAN_RX_MBOX     6
#define CAN_OVF_MBOX    6
#define CAN_MBOXES      NB_MOB_CHANNEL

#if (CAN_RX_MBOX + CAN_TX_MBOX + CAN_OVF_MBOX) != CAN_MBOXES
#error Invalid mailbox config
#endif

#define CAN_TX_FIRST    0
#define CAN_TX_LAST    (CAN_TX_FIRST + CAN_TX_MBOX - 1)
#define CAN_RX_FIRST   (CAN_TX_LAST + 1)
#define CAN_RX_LAST    (CAN_RX_FIRST + CAN_RX_MBOX - 1)
#define CAN_OVF_FIRST  (CAN_RX_LAST + 1)
#define CAN_OVF_LAST   (CAN_OVF_FIRST + CAN_OVF_MBOX - 1)

#define NB_CAN_CHANNELS 2

#define CAN_CMD_REFUSED          0xFF
#define CAN_CMD_ACCEPTED         0x00
#define CAN_STATUS_COMPLETED     0x00
#define CAN_STATUS_NOT_COMPLETED 0x01
#define CAN_STATUS_ERROR         0x02
#define CAN_STATUS_WAKEUP        0x03
#define CAN_STATUS_BUSOFF        0x04
#define CAN_MOB_NOT_ALLOCATED    0xFF

typedef struct {
    // Rx callback - current implementation only supports one callback per device
    gs_can_rxdata_callback_t rx_callback;
    void * rx_user_data;

    uint32_t can_bitrate;
    uint8_t can_operating_mode;

    uint32_t mob_alloc_vector;
    uint8_t next_tx_mob;

    SemaphoreHandle_t mob_tx_wake;
    SemaphoreHandle_t mob_tx_lock;

    bool rx_overflow_check;
    uint32_t active_mobs;

    uint32_t multiple_rx_frames;
    uint32_t can_rx_irqs;
    uint32_t can_tx_irqs;
    uint32_t can_er_irqs;
    uint32_t can_bo_irqs;
    uint32_t can_wk_irqs;
    uint32_t tx_timeout;
    uint32_t tx_full;

} can_driver_if_inst_t;

static volatile can_msg_t mob_ram_ch[NB_CAN_CHANNELS][NB_MOB_CHANNEL];
static can_driver_if_inst_t can_if[NB_CAN_CHANNELS];

// forward declarations
static void can_init_interrupt(uint8_t can_ch);

static uint8_t can_mob_alloc(uint8_t can_ch, uint8_t mob)
{
    if ((can_ch >= NB_CAN_CHANNELS) || (mob >= NB_MOB_CHANNEL)) {
        return CAN_CMD_REFUSED;
    }
    if (!((can_if[can_ch].mob_alloc_vector >> mob) & 0x01)) {
        can_if[can_ch].mob_alloc_vector |= (1 << mob);
        CANIF_clr_mob(can_ch, mob);
        return mob;
    }
    return CAN_CMD_REFUSED;
}

static uint8_t can_mob_free_isr(uint8_t can_ch, uint8_t mob, portBASE_TYPE *task_woken)
{
    if ((can_ch >= NB_CAN_CHANNELS) || (mob >= NB_MOB_CHANNEL)) {
        return CAN_CMD_REFUSED;
    }
    if (!((can_if[can_ch].mob_alloc_vector >> mob) & 0x01)) {
        return CAN_CMD_REFUSED;
    }
    can_if[can_ch].mob_alloc_vector &= (~(1 << mob));

    /* If last mob, notify any waiting tasks */
    if (mob == CAN_TX_LAST) {
        if (xSemaphoreGiveFromISR(can_if[can_ch].mob_tx_wake, task_woken) != pdPASS) {
            log_error_isr("xSemaphoreGiveFromISR on mob_tx_wake failed (%lu)",
                      uxQueueMessagesWaiting((QueueHandle_t)can_if[can_ch].mob_tx_wake));
        }
    }

    return CAN_CMD_ACCEPTED;
}

/*
 * Lower MOBs have higher priority, so allocate MOBs from lowest to highest.
 * When we allocate the last MOB (lowest priority), wait for all transmissions
 * to succeed before allocating the first MOB (highest priority).
 */
static uint8_t can_mob_tx_alloc(uint8_t can_ch)
{
    uint8_t mob;

    /* Allocate TX mob with wrap-around */
    portENTER_CRITICAL();
    mob = can_mob_alloc(can_ch, can_if[can_ch].next_tx_mob);
    if (mob != CAN_CMD_REFUSED) {
        can_if[can_ch].next_tx_mob++;
        if (can_if[can_ch].next_tx_mob > CAN_TX_LAST)
            can_if[can_ch].next_tx_mob = CAN_TX_FIRST;
    }
    portEXIT_CRITICAL();

    return mob;
}

gs_error_t gs_can_init(uint8_t can_ch, uint32_t brate)
{
    gs_log_group_register(LOG_DEFAULT);

    log_debug("%s[%u]: bps=%"PRIu32, __FUNCTION__, can_ch, brate);
    GS_CHECK_HANDLE(can_ch < NB_CAN_CHANNELS);

    can_if[can_ch].can_bitrate = brate;
    can_if[can_ch].can_operating_mode = CANIF_CHANNEL_MODE_NORMAL;
    can_if[can_ch].active_mobs = ~0U;
    can_if[can_ch].next_tx_mob = CAN_TX_FIRST;
    can_if[can_ch].rx_overflow_check = false;

    can_if[can_ch].mob_tx_wake = xSemaphoreCreateBinary();
    can_if[can_ch].mob_tx_lock = xSemaphoreCreateMutex();
    if (!can_if[can_ch].mob_tx_wake || !can_if[can_ch].mob_tx_lock) {
        log_error("%s[%u]: alloc semaphore", __FUNCTION__, can_ch);
        return GS_ERROR_ALLOC;
    }

    /* Initialize CAN channel */
    CANIF_set_reset(can_ch);
    uint8_t breaker = 0;
    while (CANIF_channel_enable_status(can_ch)) {
        if (breaker > 10) {
            log_error("%s[%u]: enable status did not reach 0 within 100 us", __FUNCTION__, can_ch);
            return GS_ERROR_TIMEOUT;
        }
        delay_us(10);
        breaker++;
    }
    CANIF_clr_reset(can_ch);

    /* Set MOb working memory */
    CANIF_set_ram_add(can_ch,(unsigned long)&mob_ram_ch[can_ch][0]);
    /* Init bit timing */
    if ((CANIF_bit_timing(can_ch)) == 0) {
        log_error("%s[%u]: failed to set bit timing", __FUNCTION__, can_ch);
        return GS_ERROR_IO;
    }
    /* Tbit = (PRS + PHS1 + PHS2 + 4) x (PRES + 1) x PGCLK_CANIF
     * where PGCLK_CANIF is the same as OSC0 frequency */
    uint32_t pres = CANIF_get_pres(can_ch);
    switch (can_if[can_ch].can_bitrate)
    {
	case 1000000:
            pres = 1;
            break;
	case 500000:
            pres = 3;
            break;
	case 250000:
            pres = 7;
            break;
	default:
            log_error("%s[%u]: Illegal CAN bit rate: %"PRIu32"", __FUNCTION__, can_ch, can_if[can_ch].can_bitrate);
            return GS_ERROR_NOT_SUPPORTED;
    }
    CANIF_set_pres(can_ch, pres);

    pres = CANIF_get_pres(can_ch);
    uint32_t phs1 = CANIF_get_phs1(can_ch);
    uint32_t phs2 = CANIF_get_phs2(can_ch);
    uint32_t prs = CANIF_get_prs(can_ch);
    uint32_t bit_rate = BOARD_OSC0_HZ / ((prs + phs1 + phs2 + 4) * (pres + 1));
    log_debug("%s[%u]: bit_rate: %lu osc: %d phs1:%lu phs2:%lu pres:%lu prs:%lu sjw:%lu sm:%lu",
              __FUNCTION__, can_ch, bit_rate, BOARD_OSC0_HZ, phs1, phs2, pres, prs, CANIF_get_sjw(can_ch), CANIF_get_sm(can_ch));
    if ((bit_rate > 1000000) || (bit_rate < 250000)) {
        log_warning("%s[%u]: Non-standard CAN bit rate: %"PRIu32" mbps", __FUNCTION__, can_ch, bit_rate);
    }
    /* Set operating mode */
    switch (can_if[can_ch].can_operating_mode)
    {
	case CANIF_CHANNEL_MODE_NORMAL:
            CANIF_set_channel_mode(can_ch, 0);
            CANIF_clr_overrun_mode(can_ch);
            break;
	case CANIF_CHANNEL_MODE_LISTENING:
            CANIF_set_channel_mode(can_ch, 1);
            CANIF_set_overrun_mode(can_ch);
            break;
	case CANIF_CHANNEL_MODE_LOOPBACK:
            CANIF_set_channel_mode(can_ch, 2);
            CANIF_clr_overrun_mode(can_ch);
            break;
    }
    /* Initialise all MOb's and enable CAN channel */
    canif_clear_all_mob(can_ch, NB_MOB_CHANNEL);
    CANIF_enable(can_ch);

    /*
     * The maximum delay time to wait is the time to transmit 128-bits
     * (CAN extended frame at baudrate speed configured by the user).
     * - 10x bits number of previous undetected message,
     * - 128x bits MAX length,
     * - 3x bits of interframe.
     */
#define DELAY_HZ         (BAUDRATE_HZ / 141.0)   /*Compute Maximum delay time*/
#define DELAY            (1000000 / DELAY_HZ)  /*Compute Delay in us*/
    delay_us(DELAY);
    if (!CANIF_channel_enable_status(can_ch)) {
        log_error("%s[%u]: enable status did not reach 1 within %f us", __FUNCTION__, can_ch, DELAY);
        return GS_ERROR_TIMEOUT;
    }

    log_debug("%s[%u]: complete (breaker: %u)", __FUNCTION__, can_ch, breaker);

    return GS_OK;
}

gs_error_t gs_can_init_w_fallback(uint8_t device, uint32_t bps, uint32_t fallback_bps)
{
    gs_error_t error = gs_can_init(device, bps);
    if ((error == GS_ERROR_NOT_SUPPORTED) && fallback_bps) {
        error = gs_can_init(device, fallback_bps);
        if (error == GS_OK) {
            log_notice("%s[%u]: used fallback bps=%"PRIu32" -> %"PRIu32, __FUNCTION__, device, bps, fallback_bps);
        }
    }
    return error;
}


gs_error_t gs_can_start(uint8_t can_ch)
{
    GS_CHECK_HANDLE(can_ch < NB_CAN_CHANNELS);

    /* Enable interrupts */
    can_init_interrupt(can_ch);
    /* Enable all error conditions */
    AVR32_CANIF.channel[can_ch].canier |= (
        AVR32_CANIF_CANIER_AERRIM_MASK |
        AVR32_CANIF_CANIER_BERRIM_MASK |
        AVR32_CANIF_CANIER_CERRIM_MASK |
        AVR32_CANIF_CANIER_FERRIM_MASK |
        AVR32_CANIF_CANIER_SERRIM_MASK);

    log_debug("%s[%u]: canisr: %08lX cansr: %08lX cancfg: %08lX canctrl: %08lX canfc: %08lX canimr: %08lX",
              __FUNCTION__, can_ch,
              AVR32_CANIF.channel[can_ch].canisr, AVR32_CANIF.channel[can_ch].cansr,
              AVR32_CANIF.channel[can_ch].cancfg, AVR32_CANIF.channel[can_ch].canctrl,
              AVR32_CANIF.channel[can_ch].canfc, AVR32_CANIF.channel[can_ch].canimr);

    return GS_OK;
}

gs_error_t gs_can_set_extended_filter_mask(uint8_t can_ch, uint32_t canExtMsgId, uint32_t mask, gs_can_rxdata_callback_t rx_callback, void * rx_user_data)
{
    GS_CHECK_HANDLE(can_ch < NB_CAN_CHANNELS);

    can_driver_if_inst_t * pinst = &can_if[can_ch];

    pinst->rx_callback = rx_callback;
    pinst->rx_user_data = rx_user_data;

    /* Allocate and configure MOb's for RX (CAN_RX_FIRST -> CAN_OVF_LAST) */
    for (uint8_t i = CAN_RX_FIRST; i <= CAN_OVF_LAST; i++) {
        uint8_t mob = can_mob_alloc(can_ch, i);
        CANIF_set_ext_id(can_ch, mob, canExtMsgId);
        CANIF_set_ext_idmask(can_ch, mob, mask);
        CANIF_set_idemask(can_ch, mob);
        CANIF_clr_rtrmask(can_ch, mob);
        CANIF_mob_clr_automode(can_ch, mob);
        CANIF_mob_clr_dlc(can_ch, mob);
        CANIF_config_rx(can_ch, mob);
        CANIF_mob_enable(can_ch, mob);
        CANIF_mob_enable_interrupt(can_ch, mob);
    }

    return GS_OK;
}

gs_error_t gs_can_send_extended(uint8_t can_ch, uint32_t id, const void * data, size_t dlc, int timeout_ms)
{
    can_driver_if_inst_t * cif = &can_if[can_ch];

    if (cif->mob_tx_lock == 0) {
        log_error("%s[%u]: device not initialized or not available", __FUNCTION__, can_ch);
        return GS_ERROR_HANDLE;
    }

    if (xSemaphoreTake(cif->mob_tx_lock, gs_freertos_convert_ms_to_ticks(timeout_ms)) != pdPASS) {
        ++cif->tx_timeout;
        return GS_ERROR_TIMEOUT;
    }

    uint8_t mob = can_mob_tx_alloc(can_ch);
    if (mob == CAN_CMD_REFUSED) {
        /* No free MOb available */
        ++cif->tx_full;
        xSemaphoreGive(cif->mob_tx_lock);
        return GS_ERROR_FULL;
    }

    /* Set ext id, dlc and copy data to MOb */
    CANIF_set_ext_id(can_ch, mob, id);
    CANIF_set_ext_idmask(can_ch, mob, 0);
    CANIF_mob_clr_dlc(can_ch, mob);
    CANIF_mob_set_dlc(can_ch, mob, dlc);
    memcpy((CANIF_mob_get_ptr_data(can_ch, mob))->data.u8, data, dlc);
    /* Configure MOb for TX */
    CANIF_config_tx(can_ch, mob);
    /* Enable MOb and interrupt to trigger TX */
    CANIF_mob_enable(can_ch, mob);
    CANIF_mob_enable_interrupt(can_ch, mob);

    /* If allocated the last mob, wait for all transmissions to
     * finish before releasing the lock */
    if (mob == CAN_TX_LAST) {
        if (xSemaphoreTake(cif->mob_tx_wake, portMAX_DELAY) != pdPASS) {
            log_error("%s[%u]: timeout on mob_tx_wake failed (%lu)",
                      __FUNCTION__, can_ch, uxQueueMessagesWaiting(cif->mob_tx_wake));
        }
    }

    xSemaphoreGive(cif->mob_tx_lock);

    return GS_OK;
}

__attribute__((__noinline__))
static portBASE_TYPE do_can_int_tx_handler(uint8_t can_ch)
{
    portBASE_TYPE task_woken = pdFALSE;

    can_if[can_ch].can_tx_irqs++;
    /* Get the first MOb that was transmitted */
    uint8_t mob = CANIF_mob_get_mob_txok(can_ch);
    if (mob != 0x20) {
        /* Clear TX OK and reset MOb status to ack interrupt */
        CANIF_mob_clear_txok_status(can_ch, mob);
        CANIF_mob_clear_status(can_ch, mob);
        /* Free MOb */
        can_mob_free_isr(can_ch, mob, &task_woken);
    }

    return task_woken;
}

static void can_mbox_rx_enable(uint8_t can_ch, uint8_t mob)
{
    /* Re-enable MOb to enable reception of new MOb */
    CANIF_mob_enable(can_ch, mob);
    CANIF_mob_enable_interrupt(can_ch, mob);
}

static void can_mbox_rx_enable_active(uint8_t can_ch)
{
    uint8_t mob;

    for (mob = CAN_RX_FIRST; mob <= CAN_OVF_LAST; mob++) {
        if (can_if[can_ch].active_mobs & (1 << mob))
            can_mbox_rx_enable(can_ch, mob);
    }
}

static int can_mbox_rx(uint8_t can_ch, uint8_t mob, gs_context_switch_t * cswitch)
{
    if (!CANIF_mob_get_rxok_status(can_ch, mob))
        return 0;

    /* Clear RX OK and reset MOb status to ack interrupt */
    CANIF_mob_clear_rxok_status(can_ch, mob);
    CANIF_mob_clear_status(can_ch, mob);

    can_msg_t *cm = CANIF_mob_get_ptr_data(can_ch, mob);

    can_driver_if_inst_t * pinst = &can_if[can_ch];

    pinst->rx_callback(can_ch,
                       CANIF_get_ext_id(can_ch, mob),
                       true,
                       cm->data.u8,
                       CANIF_mob_get_dlc(can_ch, mob),
                       0,
                       pinst->rx_user_data,
                       cswitch);

    return 1;
}

__attribute__((__noinline__))
static portBASE_TYPE do_can_int_rx_handler(uint8_t can_ch)
{
    gs_context_switch_t cswitch = GS_CONTEXT_SWITCH_INIT;
    uint8_t received = 0;
    uint8_t mob;

    can_if[can_ch].can_rx_irqs++;
    /* Check overflow mailboxes first */
    if (can_if[can_ch].rx_overflow_check) {
        can_if[can_ch].rx_overflow_check = false;
        for (mob = CAN_OVF_FIRST; mob <= CAN_OVF_LAST; mob++) {
            received += can_mbox_rx(can_ch, mob, &cswitch);
            can_if[can_ch].active_mobs |= (1 << mob);
            can_mbox_rx_enable(can_ch, mob);
        }
        if (received > 1) {
            log_info_isr("Got %u frames from OVF in one interrupt", received);
        }
        received = 0;
    }

    /* Check RX & and overflow mailboxes */
    for (mob = CAN_RX_FIRST; mob <= CAN_OVF_LAST; mob++) {
        /* Skip if not active */
        if (!(can_if[can_ch].active_mobs & (1 << mob)))
            continue;

        if (!can_mbox_rx(can_ch, mob, &cswitch))
            break;

        can_if[can_ch].active_mobs &= ~(1 << mob);
        received++;
    }
    if (received > 1)
        can_if[can_ch].multiple_rx_frames++;

    /* If we used overflow MObs, enable regular RX and scan overflows first */
    if (mob >= CAN_OVF_FIRST) {
        can_if[can_ch].active_mobs = ~0U;
        can_mbox_rx_enable_active(can_ch);
        can_if[can_ch].rx_overflow_check = true;
    }

    if (mob >= CAN_OVF_LAST) {
        log_error_isr("overflow: %u\n", mob);
    }

    return cswitch.task_woken;
}

__attribute__((__noinline__))
static portBASE_TYPE do_can_int_busoff_handler(uint8_t can_ch)
{
    portBASE_TYPE task_woken = pdFALSE;

    can_if[can_ch].can_bo_irqs++;
    uint8_t mob = CANIF_get_interrupt_lastmob_selected(can_ch);
    if (mob != 0x20) {
        if (CANIF_mob_get_dir(can_ch, mob) == 1) {
            /* Clear TX OK and reset MOb status to ack interrupt */
            CANIF_mob_clear_txok_status(can_ch, mob);
            CANIF_mob_clear_status(can_ch, mob);
            /* Disable and free MOb */
            CANIF_mob_disable(can_ch, mob);
            can_mob_free_isr(can_ch, mob, &task_woken);
        }
    }
    CANIF_clr_interrupt_status(can_ch);

    return task_woken;
}

/*
 * From avrfreaks.net:
 * The CANSTMOB AERR (Acknowledge error) is unique (according to the Bosch
 * CAN specification). Any other CAN Tx error will increment CANTEC up to
 * the bus off error confinement state (255 then an overflow).
 * However, AERR all by itself will not increment CANTEC after it reaches
 * error passive (128). When the CANSTMOB RXOK and TXOK flags are set,
 * the CAN hardware will clear all the BERR, SERR, CERR, FERR and AERR
 * error flags. All of these CANSTMOB error flag values are temporary unless
 * your Tx never works at all. If you do get a TXOK flag, then the Tx did work.
 * Since AERR all by itself never triggers bus off error confinement,
 * it just keeps sending Tx retries forever until a TXOK or until your
 * software shuts it down (except for TTC mode).
 * The Tx AERR usually means your CAN node is disconnected from the CAN network,
 * which probably explains why Bosch did not allow it to trigger a bus off.
 */
__attribute__((__noinline__))
static portBASE_TYPE do_can_int_error_handler(uint8_t can_ch)
{
    portBASE_TYPE task_woken = pdFALSE;

    can_if[can_ch].can_er_irqs++;
    uint8_t mob = CANIF_get_interrupt_lastmob_selected(can_ch);
    if (mob != 0x20) {
        if (CANIF_mob_get_dir(can_ch, mob) == 1) {
            /* Clear TX OK and reset MOb status to ack interrupt */
            CANIF_mob_clear_txok_status(can_ch, mob);
            CANIF_mob_clear_status(can_ch, mob);
            /* Disable and free MOb */
            CANIF_mob_disable(can_ch, mob);
            can_mob_free_isr(can_ch, mob, &task_woken);
        }
    }
    CANIF_clr_interrupt_status(can_ch);

    return task_woken;
}

__attribute__((__naked__))
void can_int_tx_handler0(void)
{
    portENTER_SWITCHING_ISR();
    do_can_int_tx_handler(0);
    portEXIT_SWITCHING_ISR();
}

__attribute__((__naked__))
void can_int_tx_handler1(void)
{
    portENTER_SWITCHING_ISR();
    do_can_int_tx_handler(1);
    portEXIT_SWITCHING_ISR();
}

__attribute__((__naked__))
void can_int_rx_handler0(void)
{
    portENTER_SWITCHING_ISR();
    do_can_int_rx_handler(0);
    portEXIT_SWITCHING_ISR();
}

__attribute__((__naked__))
void can_int_rx_handler1(void)
{
    portENTER_SWITCHING_ISR();
    do_can_int_rx_handler(1);
    portEXIT_SWITCHING_ISR();
}

__attribute__((__naked__))
void can_int_busoff_handler0(void)
{
    portENTER_SWITCHING_ISR();
    do_can_int_busoff_handler(0);
    portEXIT_SWITCHING_ISR();
}

__attribute__((__naked__))
void can_int_busoff_handler1(void)
{
    portENTER_SWITCHING_ISR();
    do_can_int_busoff_handler(1);
    portEXIT_SWITCHING_ISR();
}

__attribute__((__naked__))
void can_int_error_handler0(void)
{
    portENTER_SWITCHING_ISR();
    do_can_int_error_handler(0);
    portEXIT_SWITCHING_ISR();
}

__attribute__((__naked__))
void can_int_error_handler1(void)
{
    portENTER_SWITCHING_ISR();
    do_can_int_error_handler(1);
    portEXIT_SWITCHING_ISR();
}

__attribute__((__interrupt__))
static void can_int_wakeup_handler0(void)
{
    CANIF_clr_interrupt_status(0);
}

__attribute__((__interrupt__))
static void can_int_wakeup_handler1(void)
{
    CANIF_clr_interrupt_status(1);
}

static void can_init_interrupt(uint8_t can_ch)
{
    if (can_ch == 0) {
        INTC_register_interrupt(&can_int_tx_handler0, AVR32_CANIF_TXOK_IRQ_0, CAN0_INT_TX_LEVEL);
        INTC_register_interrupt(&can_int_rx_handler0, AVR32_CANIF_RXOK_IRQ_0, CAN0_INT_RX_LEVEL);
        INTC_register_interrupt(&can_int_busoff_handler0, AVR32_CANIF_BUS_OFF_IRQ_0, CAN0_INT_BOFF_LEVEL);
        INTC_register_interrupt(&can_int_error_handler0, AVR32_CANIF_ERROR_IRQ_0, CAN0_INT_ERR_LEVEL);
        INTC_register_interrupt(&can_int_wakeup_handler0, AVR32_CANIF_WAKE_UP_IRQ_0, CAN0_INT_WAKE_UP_LEVEL);
        CANIF_enable_interrupt(can_ch);
    } else if (can_ch == 1) {
        INTC_register_interrupt(&can_int_tx_handler1, AVR32_CANIF_TXOK_IRQ_1, CAN1_INT_TX_LEVEL);
        INTC_register_interrupt(&can_int_rx_handler1, AVR32_CANIF_RXOK_IRQ_1, CAN1_INT_RX_LEVEL);
        INTC_register_interrupt(&can_int_busoff_handler1, AVR32_CANIF_BUS_OFF_IRQ_1, CAN1_INT_BOFF_LEVEL);
        INTC_register_interrupt(&can_int_error_handler1, AVR32_CANIF_ERROR_IRQ_1, CAN1_INT_ERR_LEVEL);
        INTC_register_interrupt(&can_int_wakeup_handler1, AVR32_CANIF_WAKE_UP_IRQ_1, CAN1_INT_WAKE_UP_LEVEL);
        CANIF_enable_interrupt(can_ch);
    }
}

gs_error_t gs_can_driver_print_stats(FILE * out, uint8_t can_ch)
{
    GS_CHECK_HANDLE(can_ch < NB_CAN_CHANNELS);

    const can_driver_if_inst_t * cif = &can_if[can_ch];

    fprintf(out, "CAN interface: %u, bps: %"PRIu32", mode: %u\r\n", can_ch, cif->can_bitrate, cif->can_operating_mode);
    fprintf(out, "Rx callback: %p, %p\r\n", cif->rx_callback, cif->rx_user_data);

    fprintf(out, "Active MOBs: 0x%"PRIx32"\r\n", cif->active_mobs);
    fprintf(out, "Alloc MOBs:  0x%"PRIx32"\r\n", cif->mob_alloc_vector);
    fprintf(out, "TX Mobs  %02u -> %02u\r\n", CAN_TX_FIRST, CAN_TX_LAST);
    fprintf(out, "RX Mobs  %02u -> %02u\r\n", CAN_RX_FIRST, CAN_RX_LAST);
    fprintf(out, "OV Mobs: %02u -> %02u\r\n", CAN_OVF_FIRST, CAN_OVF_LAST);
    fprintf(out, "RX ovfc: %d\r\n", cif->rx_overflow_check);
    fprintf(out, "IRQ: RX: %"PRIu32" TX: %"PRIu32" ER: %"PRIu32" BO: %"PRIu32" WK: %"PRIu32"\r\n",
            cif->can_rx_irqs, cif->can_tx_irqs,
            cif->can_er_irqs, cif->can_bo_irqs, cif->can_wk_irqs);
    fprintf(out, "Multiple RX frames: %"PRIu32"\r\n", cif->multiple_rx_frames);
    fprintf(out, "TX (dropped): lock timeout: %"PRIu32" full/no free MOBs: %"PRIu32"\r\n",
            cif->tx_timeout, cif->tx_full);
    fprintf(out, "CANFC: EMODE:%lu TEC:%lu REC:%lu\r\n", CANIF_get_error_mode(can_ch), CANIF_get_tec(can_ch), CANIF_get_rec(can_ch));

    return GS_OK;
}

gs_error_t gs_can_print_driver_stats(uint8_t device)
{
    return gs_can_driver_print_stats(stdout, device);
}
