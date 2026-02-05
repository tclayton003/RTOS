/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
 * CAN driver
 */

/* --- includes ----------------------------------------------------------*/

#include <gs/embed/asf/samc/drivers/can/can.h>
#include <gs/util/drivers/can/can.h>
#include <gs/util/time.h>
#include <gs/util/thread.h>
#include <gs/util/check.h>
#include <gs/embed/asf/samc/convert_asf_status_code.h>
#include <gs/embed/asf/samc/drivers/gpio.h>
#include <gs/embed/freertos.h>
#include <gs/embed/asf/samc/command.h>
#include <gs/util/gosh/command.h>

// libasf
#include <can.h>
#include <system_interrupt.h>

#include <string.h>

/* --- definitions -------------------------------------------------------*/

#define LOG_DEFAULT gs_can_log

GS_CHECK_STATIC_ASSERT(CONF_CAN0_RX_STANDARD_ID_FILTER_NUM == CONF_CAN0_RX_EXTENDED_ID_FILTER_NUM, difference_in_max_filter);

// SAMC21e18a has only pins for CAN0 - CAN1 cannot be mapped to any pins.
#define MAX_CAN_HANDLES    1
#define DEFAULT_BPS        1000000
#define MAX_FILTER_INDEX   CONF_CAN0_RX_STANDARD_ID_FILTER_NUM

typedef enum {
    GS_CAN_NOT_INITIALIZED = 0,
    GS_CAN_DISABLED,
    GS_CAN_STOPPED,
    GS_CAN_STARTED
} gs_can_state_t;

typedef struct {
    gs_can_rxdata_callback_t callback;
    void * user_data;
} gs_can_rx_callback_t;

typedef struct {
    uint8_t filter_index;
    gs_can_rx_callback_t callback[MAX_FILTER_INDEX];
} gs_can_rx_t;

typedef enum {
    GS_CAN_ID_TYPE_STANDARD = 0,
    GS_CAN_ID_TYPE_EXTENDED = 1,
    GS_CAN_ID_TYPE_MAX = 2
} gs_can_id_type_t;

typedef struct {
    uint32_t rx;
    uint32_t tx;
    uint32_t tx_full;
} gs_can_frames_stat_t;

typedef struct
{
    gs_can_state_t state;

    // GS CAN configuration
    struct gs_can_config config;

    // ATMEL configuration
    struct can_module instance;

    // CAN stats
    struct {
        gs_can_frames_stat_t frames[GS_CAN_ID_TYPE_MAX];
        uint32_t interrupt_events;
        bool isr_clear_stats; // signal to ISR to clear values
    } stats;

    // callbacks for received data
    gs_can_rx_t rx[GS_CAN_ID_TYPE_MAX];

} can_handle_t;

static can_handle_t can_handles[MAX_CAN_HANDLES];

static bool debug_can;

/* --- code --------------------------------------------------------------*/

static inline can_handle_t * get_handle(uint8_t hdl)
{
    if (hdl < MAX_CAN_HANDLES) {
        return &can_handles[hdl];
    }
    return NULL;
}

static inline uint32_t gs_can_normalize_id(uint32_t id, uint32_t xtd)
{
    if (xtd) {
        return id;
    }

    return ((id & CAN_TX_ELEMENT_T0_STANDARD_ID_Msk) >> CAN_TX_ELEMENT_T0_STANDARD_ID_Pos);
}

gs_error_t gs_can_set_standard_filter_mask(uint8_t hdl, uint32_t canMsgId, uint32_t mask, gs_can_rxdata_callback_t rx_callback, void * rx_user_data)
{
    can_handle_t * handle = get_handle(hdl);
    if (handle == NULL) {
        return GS_ERROR_HANDLE;
    }

    gs_can_rx_t * rx = &handle->rx[GS_CAN_ID_TYPE_STANDARD];

    if (rx->filter_index >= MAX_FILTER_INDEX) {
        log_error("%s: too many filters, max is %u",
                  __FUNCTION__, MAX_FILTER_INDEX);
        return GS_ERROR_FULL;
    }

    struct can_standard_message_filter_element filter;
    can_get_standard_message_filter_element_default(&filter);

    filter.S0.bit.SFID1 = canMsgId;
    filter.S0.bit.SFID2 = mask;

    if (debug_can) {
        log_debug("%s: SFID1=%u, SFID2=%u, SFEC=%u, SFT=%u",
                  __FUNCTION__, (unsigned int)filter.S0.bit.SFID1, (unsigned int)filter.S0.bit.SFID2, (unsigned int)filter.S0.bit.SFEC, (unsigned int)filter.S0.bit.SFT);
    }

    enum status_code status = can_set_rx_standard_filter(&handle->instance, &filter, rx->filter_index);
    if (status == STATUS_OK) {
        can_enable_interrupt(&handle->instance, CAN_RX_FIFO_0_NEW_MESSAGE);
        rx->callback[rx->filter_index].callback = rx_callback;
        rx->callback[rx->filter_index].user_data = rx_user_data;
        rx->filter_index++;
    }

    return gs_convert_asf_status_code(status);
}

gs_error_t gs_can_set_extended_filter_mask(uint8_t hdl, uint32_t canExtMsgId, uint32_t mask, gs_can_rxdata_callback_t rx_callback, void * rx_user_data)
{
    can_handle_t * handle = get_handle(hdl);
    if (handle == NULL) {
        return GS_ERROR_HANDLE;
    }

    gs_can_rx_t * rx = &handle->rx[GS_CAN_ID_TYPE_EXTENDED];

    if (rx->filter_index >= MAX_FILTER_INDEX) {
        log_error("%s: too many filters, max is %u",
                  __FUNCTION__, MAX_FILTER_INDEX);
        return GS_ERROR_FULL;
    }

    struct can_extended_message_filter_element filter;
    can_get_extended_message_filter_element_default(&filter);

    filter.F0.bit.EFID1 = canExtMsgId;
    filter.F1.bit.EFID2 = mask;

    if (debug_can) {
        log_debug("%s: EFID1=%u, EFID2=%u, EFEC=%u, EFT=%u",
                  __FUNCTION__, (unsigned int)filter.F0.bit.EFID1, (unsigned int)filter.F1.bit.EFID2, (unsigned int)filter.F0.bit.EFEC, (unsigned int)filter.F1.bit.EFT);
    }

    enum status_code status = can_set_rx_extended_filter(&handle->instance, &filter, rx->filter_index);
    if (status == STATUS_OK) {
        can_enable_interrupt(&handle->instance, CAN_RX_FIFO_1_NEW_MESSAGE);
        rx->callback[rx->filter_index].callback = rx_callback;
        rx->callback[rx->filter_index].user_data = rx_user_data;
        rx->filter_index++;
    }

    return gs_convert_asf_status_code(status);
}

static inline gs_error_t _return_error_restart(gs_error_t error, bool restart, bool * restart_required)
{
    if (restart_required) {
        *restart_required = restart;
    }
    return error;
}

static inline gs_error_t gs_can_int_error_state(can_handle_t * handle, bool * restart_required)
{
    // Disabled
    if (handle->state == GS_CAN_DISABLED) {
        return _return_error_restart(GS_ERROR_ACCESS, false, restart_required);
    }

    CAN_PSR_Type psr;
    psr.reg = handle->instance.hw->PSR.reg;
    // bus error
    if (psr.bit.BO) {
        return _return_error_restart(GS_ERROR_IO, true, restart_required);
    }

    // CAN started
    CAN_CCCR_Type cccr;
    cccr.reg = handle->instance.hw->CCCR.reg;
    if (cccr.bit.INIT) {
        return _return_error_restart(GS_ERROR_ACCESS, true, restart_required);
    }
    if (cccr.bit.CCE) {
        return _return_error_restart(GS_ERROR_BUSY, true, restart_required);
    }
    if (cccr.bit.ASM) {
        return _return_error_restart(GS_ERROR_ACCESS, true, restart_required);
    }

    // TX queue full
    if (handle->instance.hw->TXFQS.bit.TFQF) {
        return _return_error_restart(GS_ERROR_FULL, false, restart_required);
    }

    // ok - no problems
    return _return_error_restart(GS_OK, false, restart_required);
}

static gs_error_t gs_can_send(uint8_t hdl, uint32_t id_value, bool extended, const void *data, size_t data_size, int timeout_ms)
{
    can_handle_t * handle = get_handle(hdl);
    if (handle == NULL) {
        return GS_ERROR_HANDLE;
    }

    // wait for free space in TX buffer
    {
        const int DELAY_MS = 3;
        gs_error_t can_error = gs_can_int_error_state(handle, NULL);
        while ((can_error == GS_ERROR_FULL) && (timeout_ms > 0)) {
            gs_thread_sleep_ms(DELAY_MS); // optimize using a semaphore
            timeout_ms -= DELAY_MS;
            can_error = gs_can_int_error_state(handle, NULL);
        }

        if (can_error) {
            return can_error;
        }
    }

    // get index of next FIFO buffer to use
    unsigned int fifo_idx = handle->instance.hw->TXFQS.bit.TFQPI;

    struct can_tx_element tx_element;
    can_get_tx_buffer_element_defaults(&tx_element);

    gs_can_frames_stat_t * stats;
    if (extended) {
        tx_element.T0.reg |= CAN_TX_ELEMENT_T0_EXTENDED_ID(id_value) | CAN_TX_ELEMENT_T0_XTD;
        stats = &handle->stats.frames[GS_CAN_ID_TYPE_EXTENDED];
    } else {
        tx_element.T0.reg |= CAN_TX_ELEMENT_T0_STANDARD_ID(id_value);
        stats = &handle->stats.frames[GS_CAN_ID_TYPE_STANDARD];
    }
    tx_element.T1.bit.DLC = data_size;
    memcpy(tx_element.data, data, data_size);

    enum status_code status = can_set_tx_buffer_element(&handle->instance, &tx_element, fifo_idx);
    if (status == STATUS_OK) {
        status = can_tx_transfer_request(&handle->instance, 1 << fifo_idx);
    }

    gs_error_t error = gs_convert_asf_status_code(status);
    if (error == GS_ERROR_FULL) {
        ++stats->tx_full;
    } else if (error == GS_OK) {
        ++stats->tx;
    }

    return error;
}

gs_error_t gs_can_error_state(uint8_t hdl, bool * restart_required)
{
    can_handle_t * handle = get_handle(hdl);
    if (handle == NULL) {
        if (restart_required) {
            *restart_required = false;
        }
        return GS_ERROR_HANDLE;
    }

    return gs_can_int_error_state(handle, restart_required);
}

gs_error_t gs_can_send_standard(uint8_t hdl, uint32_t canMsgId, const void * data, size_t data_size, int timeout_ms)
{
    return gs_can_send(hdl, canMsgId, false, data, data_size, timeout_ms);
}

gs_error_t gs_can_send_extended(uint8_t hdl, uint32_t canExtMsgId, const void * data, size_t data_size, int timeout_ms)
{
    return gs_can_send(hdl, canExtMsgId, true, data, data_size, timeout_ms);
}

void gs_can_print_interrupt_events(uint32_t status, const char * indent)
{
    /** Rx FIFO 0 New Message Interrupt Enable. */
    if (status & CAN_RX_FIFO_0_NEW_MESSAGE) {
        printf("%sCAN_RX_FIFO_0_NEW_MESSAGE\r\n", indent);
    }
    /** Rx FIFO 0 Watermark Reached Interrupt Enable. */
    if (status & CAN_RX_FIFO_0_WATERMARK) {
        printf("%sCAN_RX_FIFO_0_WATERMARK\r\n", indent);
    }
    /** Rx FIFO 0 Full Interrupt Enable. */
    if (status & CAN_RX_FIFO_0_FULL) {
        printf("%sCAN_RX_FIFO_0_FULL\r\n", indent);
    }
    /** Rx FIFO 0 Message Lost Interrupt Enable. */
    if (status & CAN_RX_FIFO_0_LOST_MESSAGE) {
        printf("%sCAN_RX_FIFO_0_LOST_MESSAGE\r\n", indent);
    }
    /** Rx FIFO 1 New Message Interrupt Enable. */
    if (status & CAN_RX_FIFO_1_NEW_MESSAGE) {
        printf("%sCAN_RX_FIFO_1_NEW_MESSAGE\r\n", indent);
    }
    /** Rx FIFO 1 Watermark Reached Interrupt Enable. */
    if (status & CAN_RX_FIFO_1_WATERMARK) {
        printf("%sCAN_RX_FIFO_1_WATERMARK\r\n", indent);
    }
    /** Rx FIFO 1 Full Interrupt Enable. */
    if (status & CAN_RX_FIFO_1_FULL) {
        printf("%sCAN_RX_FIFO_1_FULL\r\n", indent);
    }
    /** Rx FIFO 1 Message Lost Interrupt Enable. */
    if (status & CAN_RX_FIFO_1_MESSAGE_LOST) {
        printf("%sCAN_RX_FIFO_1_MESSAGE_LOST\r\n", indent);
    }
    /** High Priority Message Interrupt Enable. */
    if (status & CAN_RX_HIGH_PRIORITY_MESSAGE) {
        printf("%sCAN_RX_HIGH_PRIORITY_MESSAGE\r\n", indent);
    }
    /** Timestamp Completed Interrupt Enable. */
    if (status & CAN_TIMESTAMP_COMPLETE) {
        printf("%sCAN_TIMESTAMP_COMPLETE\r\n", indent);
    }
    /** Transmission Cancellation Finished Interrupt Enable. */
    if (status & CAN_TX_CANCELLATION_FINISH) {
        printf("%sCAN_TX_CANCELLATION_FINISH\r\n", indent);
    }
    /** Tx FIFO Empty Interrupt Enable. */
    if (status & CAN_TX_FIFO_EMPTY) {
        printf("%sCAN_TX_FIFO_EMPTY\r\n", indent);
    }
    /** Tx Event FIFO New Entry Interrupt Enable. */
    if (status & CAN_TX_EVENT_FIFO_NEW_ENTRY) {
        printf("%sCAN_TX_EVENT_FIFO_NEW_ENTRY\r\n", indent);
    }
    /** Tx Event FIFO Watermark Reached Interrupt Enable. */
    if (status & CAN_TX_EVENT_FIFO_WATERMARK) {
        printf("%sCAN_TX_EVENT_FIFO_WATERMARK\r\n", indent);
    }
    /** Tx Event FIFO Full Interrupt Enable. */
    if (status & CAN_TX_EVENT_FIFO_FULL) {
        printf("%sCAN_TX_EVENT_FIFO_FULL\r\n", indent);
    }
    /** Tx Event FIFO Element Lost Interrupt Enable. */
    if (status & CAN_TX_EVENT_FIFO_ELEMENT_LOST) {
        printf("%sCAN_TX_EVENT_FIFO_ELEMENT_LOST\r\n", indent);
    }
    /** Timestamp Wraparound Interrupt Enable. */
    if (status & CAN_TIMESTAMP_WRAPAROUND) {
        printf("%sCAN_TIMESTAMP_WRAPAROUND\r\n", indent);
    }
    /** Message RAM Access Failure Interrupt Enable. */
    if (status & CAN_MESSAGE_RAM_ACCESS_FAILURE) {
        printf("%sCAN_MESSAGE_RAM_ACCESS_FAILURE\r\n", indent);
    }
    /** Timeout Occurred Interrupt Enable. */
    if (status & CAN_TIMEOUT_OCCURRED) {
        printf("%sCAN_TIMEOUT_OCCURRED\r\n", indent);
    }
    /** Message stored to Dedicated Rx Buffer Interrupt Enable. */
    if (status & CAN_RX_BUFFER_NEW_MESSAGE) {
        printf("%sCAN_RX_BUFFER_NEW_MESSAGE\r\n", indent);
    }
    /** Bit Error Corrected Interrupt Enable. */
    if (status & CAN_BIT_ERROR_CORRECTED) {
        printf("%sCAN_BIT_ERROR_CORRECTED\r\n", indent);
    }
    /** Bit Error Uncorrected Interrupt Enable. */
    if (status & CAN_BIT_ERROR_UNCORRECTED) {
        printf("%sCAN_BIT_ERROR_UNCORRECTED\r\n", indent);
    }
    /** Error Logging Overflow Interrupt Enable. */
    if (status & CAN_ERROR_LOGGING_OVERFLOW) {
        printf("%sCAN_ERROR_LOGGING_OVERFLOW\r\n", indent);
    }
    /** Error Passive Interrupt Enable. */
    if (status & CAN_ERROR_PASSIVE) {
        printf("%sCAN_ERROR_PASSIVE\r\n", indent);
    }
    /** Warning Status Interrupt Enable. */
    if (status & CAN_WARNING_STATUS) {
        printf("%sCAN_WARNING_STATUS\r\n", indent);
    }
    /** Bus_Off Status Interrupt Enable. */
    if (status & CAN_BUS_OFF) {
        printf("%sCAN_BUS_OFF\r\n", indent);
    }
    /** Watchdog Interrupt Interrupt Enable. */
    if (status & CAN_WATCHDOG) {
        printf("%sCAN_WATCHDOG\r\n", indent);
    }
    /** Protocol Error in Arbitration Phase Enable. */
    if (status & CAN_PROTOCOL_ERROR_ARBITRATION) {
        printf("%sCAN_PROTOCOL_ERROR_ARBITRATION\r\n", indent);
    }
    /** Protocol Error in Data Phase Enable. */
    if (status & CAN_PROTOCOL_ERROR_DATA) {
        printf("%sCAN_PROTOCOL_ERROR_DATA\r\n", indent);
    }
    /** Access to Reserved Address Enable. */
    if (status & CAN_ACCESS_RESERVED_ADDRESS) {
        printf("%sCAN_ACCESS_RESERVED_ADDRESS\r\n", indent);
    }
}

/**
   Process Tx Event fifo element.
*/
static void gs_can_process_tx_event_fifo(can_handle_t * handle)
{
    CAN_TXEFS_Type reg;

    reg.reg = handle->instance.hw->TXEFS.reg;
    if (reg.bit.EFFL > 0) {
        struct can_tx_event_element elm;
        enum status_code status = can_get_tx_event_fifo_element(&handle->instance, &elm, reg.bit.EFGI);
        if (status == STATUS_OK) {
#if DEBUG_PRINT_CAN_TX_EVENT_FIFO
            /* printf("Tx Event FIFO [%u] - E0: id: 0x%x -> 0x%" PRIx32 ", RTR: %u, XTD: %u, ESI: %u\r\n", */
            /* 	     reg.bit.EFGI, elm.E0.bit.ID, gs_can_normalize_id(elm.E0.bit.ID, elm.E0.bit.XTD), elm.E0.bit.RTR, elm.E0.bit.XTD, elm.E0.bit.ESI); */
            /* printf("Tx Event FIFO [%u] - E1: TXTS: %u, DLC: %u, BRS: %u, FDF: %u, ET: %u, MM: %u\r\n", */
            /* 	     reg.bit.EFGI, elm.E1.bit.TXTS, elm.E1.bit.DLC, elm.E1.bit.BRS, elm.E1.bit.FDF, elm.E1.bit.ET, elm.E1.bit.MM); */
#endif

            can_tx_event_fifo_acknowledge(&handle->instance, reg.bit.EFGI);
        }
    }
}

/**
 * RX FIFO handling. All RX FIFO 0 and 1 structures are identical - except naming contains 0 or 1.
 */
static void gs_can_process_rx_fifo_isr(can_handle_t * handle, uint8_t fifo)
{
    gs_context_switch_t cswitch = GS_CONTEXT_SWITCH_INIT;
    for (int loop = 0; loop < 100; ++loop) {
        CAN_RXF0S_Type reg;
        reg.reg = can_rx_get_fifo_status(&handle->instance, fifo);
        if (reg.bit.F0FL == 0) {
            return;
        }

        union
        {
            struct can_rx_element_fifo_0 elm;
            struct can_rx_element_fifo_1 elm1;
        } elm;
        enum status_code status;
        if (fifo == 0) {
            status = can_get_rx_fifo_0_element(&handle->instance, &elm.elm, reg.bit.F0GI);
        } else {
            status = can_get_rx_fifo_1_element(&handle->instance, &elm.elm1, reg.bit.F0GI);
        }
        if (status != STATUS_OK) {
            return;
        }

#if DEBUG_CAN_ISR_PRINTF
        printf("\r\nRx Event FIFO: fifo=%u, index: %u, R0: id: 0x%x -> 0x%" PRIx32 ", RTR: %u, XTD: %u, ESI: %u\r\n",
               fifo, reg.bit.F0GI, elm.elm.R0.bit.ID, gs_can_normalize_id(elm.elm.R0.bit.ID, elm.elm.R0.bit.XTD), elm.elm.R0.bit.RTR, elm.elm.R0.bit.XTD, elm.elm.R0.bit.ESI);
        printf("Rx Event FIFO - R1: RXTS: %u, DLC: %u, BRS: %u, FDF: %u, FIDX: %u, ANMF: %u\r\n",
               elm.elm.R1.bit.RXTS, elm.elm.R1.bit.DLC, elm.elm.R1.bit.BRS, elm.elm.R1.bit.FDF, elm.elm.R1.bit.FIDX, elm.elm.R1.bit.ANMF);
        if (elm.elm.R1.bit.DLC) {
            printf("Rx Event FiFO data: ");
            for (uint32_t i = 0; i < elm.elm.R1.bit.DLC; i++) {
                printf("  0x%x", elm.elm.data[i]);
            }
            printf("\r\n");
        }
#endif

        const gs_can_id_type_t type = (elm.elm.R0.bit.XTD) ? GS_CAN_ID_TYPE_EXTENDED : GS_CAN_ID_TYPE_STANDARD;
        ++(handle->stats.frames[type].rx);

        if (elm.elm.R1.bit.FIDX < MAX_FILTER_INDEX) {
            gs_can_rx_callback_t * rx = &handle->rx[type].callback[elm.elm.R1.bit.FIDX];
            if (rx->callback) {
                (rx->callback)(0,
                               gs_can_normalize_id(elm.elm.R0.bit.ID, elm.elm.R0.bit.XTD),
                               elm.elm.R0.bit.XTD,
                               elm.elm.data,
                               elm.elm.R1.bit.DLC,
                               gs_time_rel_ms_isr(),
                               rx->user_data,
                               &cswitch);
            }
        }

        can_rx_fifo_acknowledge(&handle->instance, fifo, reg.bit.F0GI);
    }

    portEND_SWITCHING_ISR(cswitch.task_woken);
}

void CAN0_Handler(void)
{
    can_handle_t * handle = get_handle(0);
    if (handle == NULL) {
        return;
    }

    const uint32_t events = can_read_interrupt_status(&handle->instance);
    if (handle->stats.isr_clear_stats) {
        handle->stats.isr_clear_stats = false;
        handle->stats.interrupt_events = 0;
    }
    handle->stats.interrupt_events |= events;

    if (events & CAN_RX_FIFO_0_NEW_MESSAGE) {
        gs_can_process_rx_fifo_isr(handle, 0);
    }

    if (events & CAN_RX_FIFO_1_NEW_MESSAGE) {
        gs_can_process_rx_fifo_isr(handle, 1);
    }

    if (events & CAN_TX_EVENT_FIFO_NEW_ENTRY) {
        gs_can_process_tx_event_fifo(handle);
    }

    if (events) {
        can_clear_interrupt_status(&handle->instance, events);
    }
}

/**
 * Set butrate.
 * ASF implementation is buggy, and also doesn't check for valid rates
 */
static gs_error_t gs_can_set_bps(can_handle_t * handle, const uint32_t bps)
{
    const uint32_t gclk_can_value = system_gclk_chan_get_hz(CAN0_GCLK_ID);
    if (gclk_can_value < 1000000) {
        log_error("%s: failed to get CAN clock or invaid, returned %" PRIu32 " Hz", __FUNCTION__, gclk_can_value);
        return GS_ERROR_IO;
    }

    // find prescaler value and at the same time check if bitrate is valid
    const uint32_t nsgw_value = 3;
    const uint32_t ntseg1_value = 10;
    const uint32_t ntseg2_value = 3;
    const uint32_t MAX_PRESCALER = 200;
    uint32_t prescaler;
    for (prescaler = 0; prescaler < MAX_PRESCALER; ++prescaler) {
        const uint32_t calced_gclk_can = (prescaler + 1) * bps * (nsgw_value + ntseg1_value + ntseg2_value);
        if (calced_gclk_can == gclk_can_value) {
            // valid prescaler
            break;
        }
    }

    if (prescaler >= MAX_PRESCALER) {
        log_error("%s: invalid bitrate: %" PRIu32 " bps, current clock of %" PRIu32 " Hz can't match bitrate", __FUNCTION__, bps, gclk_can_value);
        return GS_ERROR_ARG;
    }

    handle->instance.hw->NBTP.reg = CAN_NBTP_NBRP(prescaler) | CAN_NBTP_NSJW(nsgw_value) | CAN_NBTP_NTSEG1(ntseg1_value) | CAN_NBTP_NTSEG2(ntseg2_value);
    handle->instance.hw->DBTP.reg = CAN_DBTP_DBRP(prescaler) | CAN_DBTP_DSJW(nsgw_value) | CAN_DBTP_DTSEG1(ntseg1_value) | CAN_DBTP_DTSEG2(ntseg2_value);

    if (debug_can) {
        log_debug("%s: configured bitrate: %" PRIu32 " bps, CAN clock: %" PRIu32 " Hz, prescaler: %" PRIu32 , __FUNCTION__, bps, gclk_can_value, prescaler);
    }

    handle->config.bps = bps;

    return GS_OK;
}

static bool gs_can_is_enabled(can_handle_t * handle, int * pin_level)
{
    int level = -1;
    bool enabled = false;
    if (handle->config.control == GS_CAN_CONTROL_EXTERNAL_TRANSCEIVER) {
        level = port_pin_get_output_level(handle->config.gpio);
        enabled = handle->config.enable;
    } else if (handle->config.control == GS_CAN_CONTROLLED_BY_EXTERNAL_ENABLE) {
        level = port_pin_get_input_level(handle->config.gpio);
        enabled = (level == handle->config.active_gpio_level);
    } else {
        enabled = handle->config.enable;
    }

    if (pin_level) {
        *pin_level = level;
    }

    return enabled;
}

gs_error_t gs_can_set_transceiver(uint8_t hdl, bool enable)
{
    can_handle_t * handle = get_handle(hdl);
    if (handle == NULL) {
        return GS_ERROR_HANDLE;
    }

    if (handle->state <= GS_CAN_DISABLED) {
        return GS_ERROR_ACCESS;
    }

    if (handle->config.control != GS_CAN_CONTROL_EXTERNAL_TRANSCEIVER) {
        return GS_ERROR_NOT_SUPPORTED;
    }

    port_pin_set_output_level(handle->config.gpio, enable ? handle->config.active_gpio_level : !handle->config.active_gpio_level);
    return GS_OK;
}

static void gs_can_configure_gpio(can_handle_t * handle)
{
    if (handle->config.control == GS_CAN_CONTROL_EXTERNAL_TRANSCEIVER) {
        if (debug_can) {
            log_debug("%s: configuring PIN %" PRIu32 " as output for controlling external transceiver (active level: %u)", __FUNCTION__, handle->config.gpio, handle->config.active_gpio_level);
        }

        // configure pin and disable transceiver
        gs_gpio_configure_output(handle->config.gpio, !handle->config.active_gpio_level);

    } else if (handle->config.control == GS_CAN_CONTROLLED_BY_EXTERNAL_ENABLE) {
        if (debug_can) {
            log_debug("%s: configuring PIN %" PRIu32 " as input for CAN enable (active level: %u)", __FUNCTION__, handle->config.gpio, handle->config.active_gpio_level);
        }

        gs_gpio_configure_input(handle->config.gpio, PORT_PIN_PULL_NONE);
    }
}

void gs_can_get_default_config(struct gs_can_config * config)
{
    memset(config, 0, sizeof(*config));
    config->control = GS_CAN_CONTROLLED_BY_CONFIG;
    config->bps = DEFAULT_BPS;
}

bool gs_can_debug()
{
    return debug_can;
}

void gs_can_set_debug(bool value)
{
    debug_can = value;
}

gs_error_t gs_can_init(uint8_t hdl, const struct gs_can_config * can_config, bool * enabled)
{
    if (hdl >= MAX_CAN_HANDLES) {
        return GS_ERROR_HANDLE;
    }

    gs_log_group_register(LOG_DEFAULT);

    can_handle_t * handle = &can_handles[hdl];
    memset(handle, 0, sizeof(*handle));
    if (can_config) {
        handle->config = *can_config;
    } else {
        gs_can_get_default_config(&handle->config);
    }

    gs_can_configure_gpio(handle);

    if ((gs_can_is_enabled(handle, NULL) == false) || (handle->config.enable == false)) {
        handle->state = GS_CAN_DISABLED;
        if (enabled) {
            *enabled = false;
        }
        return GS_OK;
    }

    if (enabled) {
        *enabled = true;
    }

    // Set up the CAN TX pin
    {
        struct system_pinmux_config config;
        system_pinmux_get_config_defaults(&config);

        config.mux_position = MUX_PA24G_CAN0_TX;
        system_pinmux_pin_set_config(PIN_PA24G_CAN0_TX, &config);
    }

    // Set up the CAN RX pin
    {
        struct system_pinmux_config config;
        system_pinmux_get_config_defaults(&config);

        config.mux_position = MUX_PA25G_CAN0_RX;
        system_pinmux_pin_set_config(PIN_PA25G_CAN0_RX, &config);
    }

    // Initialize the module.
    {
        struct can_config config;
        can_get_config_defaults(&config);

        config.clock_source = GCLK_GENERATOR_0;
        config.automatic_retransmission = false;

        can_init(&handle->instance, CAN0, &config);
    }

    gs_error_t error = gs_can_set_bps(handle, handle->config.bps);
    if (error) {
        log_error("%s: failed to configure %" PRIu32 " bps - using default %u bps",
                  __FUNCTION__, handle->config.bps, DEFAULT_BPS);
        gs_can_set_bps(handle, DEFAULT_BPS);
    }

    if (debug_can) {
        log_debug("%s: initialized, %" PRIu32 " bps", __FUNCTION__, handle->config.bps);
    }

    /* Enable interrupts for this CAN module */
    if (hdl == 0) {
        system_interrupt_enable(SYSTEM_INTERRUPT_MODULE_CAN0);
    } else {
        system_interrupt_enable(SYSTEM_INTERRUPT_MODULE_CAN1);
    }
    can_enable_interrupt(&handle->instance, CAN_PROTOCOL_ERROR_ARBITRATION | CAN_PROTOCOL_ERROR_DATA | CAN_TX_EVENT_FIFO_NEW_ENTRY);

    handle->state = GS_CAN_STOPPED;

    return GS_OK;
}

gs_error_t gs_can_stop(uint8_t hdl)
{
    can_handle_t * handle = get_handle(hdl);
    if (handle == NULL) {
        return GS_ERROR_HANDLE;
    }

    if (handle->state < GS_CAN_STOPPED) {
        return GS_ERROR_ACCESS;
    }

    gs_can_set_transceiver(hdl, false);
    can_stop(&handle->instance);
    handle->instance.hw->CCCR.bit.CCE = 1; // clear all pending messages
    handle->instance.hw->CCCR.bit.ASM = 0; // clear restricted mode

    handle->state = GS_CAN_STOPPED;

    return GS_OK;
}

gs_error_t gs_can_start(uint8_t hdl)
{
    can_handle_t * handle = get_handle(hdl);
    if (handle == NULL) {
        return GS_ERROR_HANDLE;
    }

    if (handle->state < GS_CAN_STOPPED) {
        return GS_ERROR_ACCESS;
    }

    gs_can_set_transceiver(hdl, true);
    can_start(&handle->instance);
    handle->state = GS_CAN_STARTED;

    return GS_OK;
}

void gs_can_print_status(uint8_t hdl)
{
    can_handle_t * handle = get_handle(hdl);
    if (handle == NULL) {
        return;
    }

    {
        int pin_level;
        const bool enabled = (gs_can_is_enabled(handle, &pin_level) && handle->config.enable);
        printf("CAN %s (%d), pin_level: %d, int. state: %d\r\n", enabled ? "enabled" : "disabled", enabled, pin_level, handle->state);
    }
    {
        //__IO CAN_CCCR_Type             CCCR;        /**< \brief Offset: 0x18 (R/W 32) CC Contro
        CAN_CCCR_Type value;
        value.reg = handle->instance.hw->CCCR.reg;
        printf("CCCR   CC Control: 0x%"PRIx32", INIT: %u, CCE: %u, ASM: %u, CSA: %u CSR: %u, MON: %u\r\n",
               value.reg, value.bit.INIT, value.bit.CCE, value.bit.ASM, value.bit.CSA, value.bit.CSR, value.bit.MON);
        printf("       DAR: %u, TEST: %u, FDOE: %u, BRSE: %u, PXHD: %u, EFBI %u, TXP: %u, NSISO: %u\r\n",
               value.bit.DAR, value.bit.TEST, value.bit.FDOE, value.bit.BRSE, value.bit.PXHD, value.bit.EFBI, value.bit.TXP, value.bit.NISO);
    }
    {
        //__I  CAN_ECR_Type              ECR;         /**< \brief Offset: 0x40 (R/  32) Error Counter */
        CAN_ECR_Type value;
        value.reg = handle->instance.hw->ECR.reg;
        printf("ECR    Error counter: 0x%"PRIx32", TxEC: %u, RxEC: %u, Rx Passive: %u, Can Error Logging: %u\r\n",
               value.reg, value.bit.TEC, value.bit.REC, value.bit.RP, value.bit.CEL);
    }
    {
        //__I  CAN_PSR_Type              PSR;         /**< \brief Offset: 0x44 (R/  32) Protocol Status */
        CAN_PSR_Type value;
        value.reg = handle->instance.hw->PSR.reg;
        printf("PSR    Protocol status: 0x%"PRIx32", LEC: %u, ACT: %u, EPas.: %u, EWar.: %u\r\n",
               value.reg, value.bit.LEC, value.bit.ACT, value.bit.EP, value.bit.EW);
        printf("       BusOff: %u, DLEC: %u, RESI: %u, RBRS: %u, RFDF: %u, PXE: %u, TDCV: %u\r\n",
               value.bit.BO, value.bit.DLEC, value.bit.RESI, value.bit.RBRS, value.bit.RFDF, value.bit.PXE, value.bit.TDCV);
    }
    {
        //   __IO CAN_TEST_Type             TEST;        /**< \brief Offset: 0x10 (R/W 32) Test */
        CAN_TEST_Type value;
        value.reg = handle->instance.hw->TEST.reg;
        printf("TEST   Test: 0x%"PRIx32", LBCK: %u, TX: %u, RX: %u\r\n",
               value.reg, value.bit.LBCK, value.bit.TX, value.bit.RX);
    }
    {
        //__I  CAN_RXF0S_Type            RXF0S;       /**< \brief Offset: 0xA4 (R/  32) Rx FIFO 0 Status */
        CAN_RXF0S_Type value;
        value.reg = handle->instance.hw->RXF0S.reg;
        printf("RXF0S  Rx FIFO 0 Status: 0x%"PRIx32", fill level: %u, get index: %u, put index: %u, full: %u, lost: %u\r\n",
               value.reg, value.bit.F0FL, value.bit.F0GI, value.bit.F0PI, value.bit.F0F, value.bit.RF0L);
    }
    {
        //__I  CAN_RXF1S_Type            RXF1S;       /**< \brief Offset: 0xA4 (R/  32) Rx FIFO 1 Status */
        CAN_RXF1S_Type value;
        value.reg = handle->instance.hw->RXF1S.reg;
        printf("RXF1S  Rx FIFO 1 Status: 0x%"PRIx32", fill level: %u, get index: %u, put index: %u, full: %u, lost: %u\r\n",
               value.reg, value.bit.F1FL, value.bit.F1GI, value.bit.F1PI, value.bit.F1F, value.bit.RF1L);
    }
    {
        //__IO CAN_TXBC_Type             TXBC;        /**< \brief Offset: 0xC0 (R/W 32) Tx Buffer Configuration */
        CAN_TXBC_Type value;
        value.reg = handle->instance.hw->TXBC.reg;
        printf("TXBC   Tx Bufs start: 0x%"PRIx32", %u, Dedic. Tx Bufs: %u, FF/Q Size: %u, FF/Q mode: %u\r\n",
               value.reg, value.bit.TBSA, value.bit.NDTB, value.bit.TFQS, value.bit.TFQM);
    }
    {
        //__I  CAN_TXFQS_Type            TXFQS;       /**< \brief Offset: 0xC4 (R/  32) Tx FIFO / Queue Status */
        CAN_TXFQS_Type value;
        value.reg = handle->instance.hw->TXFQS.reg;
        printf("TXFQS  Tx FIFO / Queue Status: 0x%"PRIx32", free level: %u, get index: %u, put index: %u, full: %u\r\n",
               value.reg, value.bit.TFFL, value.bit.TFGI, value.bit.TFQPI, value.bit.TFQF);
    }
    {
        //__IO CAN_TXESC_Type            TXESC;       /**< \brief Offset: 0xC8 (R/W 32) Tx Buffer Element Size Configuration */
        CAN_TXESC_Type value;
        value.reg = handle->instance.hw->TXESC.reg;
        printf("TXESC  Tx Buffer Elm Size Conf: 0x%"PRIx32", data field size: %u\r\n",
               value.reg, value.bit.TBDS);
    }
    {
        //__I  CAN_TXBRP_Type            TXBRP;       /**< \brief Offset: 0xCC (R/  32) Tx Buffer Request Pending */
        CAN_TXBRP_Type value;
        value.reg = handle->instance.hw->TXBRP.reg;
        printf("TXBRP  Tx Buffer Request Pending: 0x%"PRIx32"\r\n",
               value.reg);
    }
    {
        //__IO CAN_TXBAR_Type            TXBAR;       /**< \brief Offset: 0xD0 (R/W 32) Tx Buffer Add Request */
        CAN_TXBRP_Type value;
        value.reg = handle->instance.hw->TXBAR.reg;
        printf("TXBAR  Tx buffer add request: 0x%"PRIx32"\r\n",
               value.reg);
    }
    {
        //__IO CAN_TXBCR_Type            TXBCR;       /**< \brief Offset: 0xD4 (R/W 32) Tx Buffer Cancellation Request */
        CAN_TXBCR_Type value;
        value.reg = handle->instance.hw->TXBCR.reg;
        printf("TXBCR  Tx buffer cancellation request: 0x%"PRIx32"\r\n",
               value.reg);
    }
    {
        //__I  CAN_TXBTO_Type            TXBTO;       /**< \brief Offset: 0xD8 (R/  32) Tx Buffer Transmission Occurred */
        CAN_TXBTO_Type value;
        value.reg = handle->instance.hw->TXBTO.reg;
        printf("TXBTO  Tx Buffer transmission occurred: 0x%"PRIx32"\r\n",
               value.reg);
    }
    {
        //__I  CAN_TXBCF_Type            TXBCF;       /**< \brief Offset: 0xDC (R/  32) Tx Buffer Cancellation Finished */
        CAN_TXBCF_Type value;
        value.reg = handle->instance.hw->TXBCF.reg;
        printf("TXBCF  Tx buffer cancellation finished: 0x%"PRIx32"\r\n",
               value.reg);
    }
    {
        //__IO CAN_TXBTIE_Type           TXBTIE;      /**< \brief Offset: 0xE0 (R/W 32) Tx Buffer Transmission Interrupt Enable */
        CAN_TXBTIE_Type value;
        value.reg = handle->instance.hw->TXBTIE.reg;
        printf("TXBTIE Tx buffer transmission interrupt enable: 0x%"PRIx32"\r\n",
               value.reg);
    }
    {
        //__IO CAN_TXBCIE_Type           TXBCIE;      /**< \brief Offset: 0xE4 (R/W 32) Tx Buffer Cancellation Finished Interrupt Enable */
        CAN_TXBCIE_Type value;
        value.reg = handle->instance.hw->TXBCIE.reg;
        printf("TXBCIE Tx buffer cancel finished int. enable: 0x%"PRIx32"\r\n",
               value.reg);
    }
    {
        //__IO CAN_TXEFC_Type            TXEFC;       /**< \brief Offset: 0xF0 (R/W 32) Tx Event FIFO Configuration */
        CAN_TXEFC_Type value;
        value.reg = handle->instance.hw->TXEFC.reg;
        printf("TXEFC  Tx Event FIFO Configuration: 0x%"PRIx32", Start Address: %u, size: %u, watermark: %u\r\n",
               value.reg, value.bit.EFSA, value.bit.EFS, value.bit.EFWM);
    }
    {
        //__I  CAN_TXEFS_Type            TXEFS;       /**< \brief Offset: 0xF4 (R/  32) Tx Event FIFO Status */
        CAN_TXEFS_Type value;
        value.reg = handle->instance.hw->TXEFS.reg;
        printf("TXEFS  Tx Event FIFO Status: 0x%"PRIx32", Fill Level: %u, Get index: %u, Put index: %u, full: %u, lost: %u\r\n",
               value.reg, value.bit.EFFL, value.bit.EFGI, value.bit.EFPI, value.bit.EFF, value.bit.TEFL);
    }
    {
        //__IO CAN_TXEFA_Type            TXEFA;       /**< \brief Offset: 0xF8 (R/W 32) Tx Event FIFO Acknowledge */
        CAN_TXEFA_Type value;
        value.reg = handle->instance.hw->TXEFA.reg;
        printf("TXEFA  Tx Event FIFO Acknowledge: 0x%"PRIx32", Index: %u\r\n",
               value.reg, value.bit.EFAI);
    }
    {
        const uint32_t int_events = handle->stats.interrupt_events;
        printf("CAN interrupt events: 0x%x\r\n", (unsigned int) int_events);
        gs_can_print_interrupt_events(int_events, "  ");
    }
    for (gs_can_id_type_t type = 0; type < GS_CAN_ID_TYPE_MAX; ++type) {
        printf("CAN %s: rx: %9" PRIu32 ", tx: %9" PRIu32 ", tx_full: %9" PRIu32 "\r\n",
               (type == GS_CAN_ID_TYPE_STANDARD) ? "standard" : "extended",
               handle->stats.frames[type].rx, handle->stats.frames[type].tx, handle->stats.frames[type].tx_full);
    }
    {
        bool restart = false;
        gs_error_t can_error = gs_can_int_error_state(handle, &restart);
        printf("CAN error state: %d (%s), restart required: %d\r\n", can_error, gs_error_string(can_error), restart);
    }
}

void gs_can_clear_stats(uint8_t hdl)
{
    can_handle_t * handle = get_handle(hdl);
    if (handle == NULL) {
        return;
    }

    handle->stats.isr_clear_stats = true;
    handle->stats.interrupt_events = 0;
    for (gs_can_id_type_t type = 0; type < GS_CAN_ID_TYPE_MAX; ++type) {
        handle->stats.frames[type].rx = 0;
        handle->stats.frames[type].tx = 0;
        handle->stats.frames[type].tx_full = 0;
    }
}

static int cmd_send_std(struct command_context * ctx)
{
    printf("Send CAN standard message......\r\n");
    const uint8_t xtreme[8] = {1,2,3,4,5,6,7,8};
    gs_error_t error = gs_can_send_standard(0, 10, xtreme, 8, 100);
    printf(" completed, error: %d\r\n", error);
    if (error) {
        return error;
    }

    gs_can_print_status(0);
    return GS_OK;
}

static int cmd_send_ext(struct command_context * ctx)
{
    printf("Send CAN extended message......\r\n");
    uint8_t xtreme[8] = {11,22,33,44,55,66,77,88};
    gs_error_t error = gs_can_send_standard(0, 100, xtreme, 8, 100);
    printf(" completed, error: %d\r\n", error);
    if (error) {
        return error;
    }
    
    gs_can_print_status(0);
    return GS_OK;
}

static int cmd_status(struct command_context * ctx)
{
    gs_can_print_status(0);
    return GS_OK;
}

static int cmd_enable(struct command_context * ctx)
{
    gs_error_t error = gs_can_set_transceiver(0, true);
    if (error) {
        return error;
    }
    gs_can_print_status(0);
    return GS_OK;
}

static int cmd_disable(struct command_context * ctx)
{
    gs_error_t error = gs_can_set_transceiver(0, false);
    if (error) {
        return error;
    }
    gs_can_print_status(0);
    return GS_OK;
}

static int cmd_restart(struct command_context * ctx)
{
    gs_can_stop(0);
    gs_error_t error = gs_can_start(0);
    if (error) {
        return error;
    }

    gs_can_print_status(0);
    return GS_OK;
}

static int cmd_stop(struct command_context * ctx)
{
    gs_error_t error = gs_can_stop(0);
    if (error) {
        return error;
    }
    gs_can_print_status(0);
    return GS_OK;
}

static int cmd_start(struct command_context * ctx)
{
    gs_error_t error = gs_can_start(0);
    if (error) {
        return error;
    }
    gs_can_print_status(0);
    return GS_OK;
}

static int cmd_set_debug(struct command_context * ctx)
{
    if (ctx->argc >= 2) {
        gs_can_set_debug(atoi(ctx->argv[1]));
    }
    printf("Debug %d\r\n", gs_can_debug());
    return GS_OK;
}

static int cmd_clear_stats(struct command_context * ctx)
{
    gs_can_clear_stats(0);
    return GS_OK;
}

static int cmd_external_loop_back(struct command_context * ctx)
{
    gs_error_t error = gs_can_stop(0);
    if (error == GS_OK) {
        can_handle_t * handle = get_handle(0);
        handle->instance.hw->CCCR.bit.TEST = 1;
        handle->instance.hw->TEST.bit.LBCK = 1;

        error = gs_can_start(0);
    }
    if (error) {
        return error;
    }
    return GS_OK;
}

static int cmd_internal_loop_back(struct command_context * ctx)
{
    gs_error_t error = gs_can_stop(0);
    if (error == GS_OK) {
        can_handle_t * handle = get_handle(0);
        handle->instance.hw->CCCR.bit.TEST = 1;
        handle->instance.hw->CCCR.bit.MON = 1;
        handle->instance.hw->TEST.bit.LBCK = 1;

        error = gs_can_start(0);
    }
    if (error) {
        return error;
    }
    return GS_OK;
}

static const gs_command_t GS_COMMAND_SUB subcmd_can_int[] = {
    {
        .name = "send_std",
        .help = "Send CAN message with standard message id",
        .handler = cmd_send_std,
    },
    {
        .name = "send_ext",
        .help = "Send CAN message with extended message id",
        .handler = cmd_send_ext,
    },
    {
        .name = "status",
        .help = "Show general status and accumulated interrrupts (interrupts events are reset)",
        .handler = cmd_status,
    },
    {
        .name = "disable",
        .help = "Disable external transceiver (if present/configured)",
        .handler = cmd_disable,
    },
    {
        .name = "enable",
        .help = "Enable external transceiver (if present/configured)",
        .handler = cmd_enable,
    },
    {
        .name = "stop",
        .help = "Stop CAN layer",
        .handler = cmd_stop,
    },
    {
        .name = "start",
        .help = "Start CAN layer",
        .handler = cmd_start,
    },
    {
        .name = "restart",
        .help = "Restart CAN driver",
        .handler = cmd_restart,
    },
    {
        .name = "debug",
        .usage = "[0|1]",
        .help = "Set debug",
        .handler = cmd_set_debug,
    },
    {
        .name = "clear_stats",
        .help = "Clear stat counters",
        .handler = cmd_clear_stats,
    },
    {
        .name = "internal_loop_back",
        .help = "Enable internal loop back",
        .handler = cmd_internal_loop_back,
    },
    {
        .name = "external_loop_back",
        .help = "Enable external loop back",
        .handler = cmd_external_loop_back,
    },
};

static const gs_command_t GS_COMMAND_ROOT cmd_can_int[] = {
    {
        .name = "can",
        .help = "CAN debug commands",
        .chain = GS_COMMAND_INIT_CHAIN(subcmd_can_int),
    }
};

gs_error_t gs_samc_register_can_commands(void)
{
    return GS_COMMAND_REGISTER(cmd_can_int);
}
