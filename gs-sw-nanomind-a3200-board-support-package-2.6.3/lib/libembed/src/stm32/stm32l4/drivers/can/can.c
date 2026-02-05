/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <stdlib.h>
#include <gs/util/drivers/can/can.h>
#include <gs/util/check.h>
#include <gs/util/time.h>
#include <gs/util/sem.h>
#include <gs/util/thread.h>
#include <stm32l4xx_hal.h>
#include "../common.h"

#define LOG_DEFAULT gs_can_log

typedef struct gs_stm32l4_can_handle {
    //! Bool true if active
    bool inuse;
    //! Lock
    gs_sem_t tx_lock;
    //! Semaphore indicating completion of transmit
    gs_sem_t tx_sync_sem;
    //! RX callback
    gs_can_rxdata_callback_t can_rx_callback;
    //! User data for the callback
    void * rx_user_data;
    //! RX buffer
    uint8_t rx_buffer[8];
} gs_stm32l4_can_handle_t;

static gs_stm32l4_can_handle_t handle;

#define TIME_QUANTA_PER_BIT     10
static gs_error_t calculate_prescaler(size_t speed, uint32_t * prescaler)
{
    GS_CHECK_RANGE(speed >= 125000 && speed <= 1000000);
    uint32_t clock_freq_hz = HAL_RCC_GetPCLK1Freq();
    uint32_t resulting_speed;
    for(int i = 1; i <= 1024; i++) {
        resulting_speed = clock_freq_hz / (i * TIME_QUANTA_PER_BIT);
        if(resulting_speed <= speed) {
            uint32_t former_resulting_speed = clock_freq_hz / ((i - 1) * TIME_QUANTA_PER_BIT);
            if (abs(resulting_speed - speed) <= abs(former_resulting_speed - speed)) {
                *prescaler = i;
            } else {
                if (i > 1) {
                    *prescaler = i - 1;
                }
                else {
                    *prescaler = i;
                }
            }
            return GS_OK;
        }
    }
    return GS_ERROR_RANGE;
}

gs_error_t gs_stm32l4_can_init(uint8_t device, size_t bps)
{
    GS_CHECK_ARG(device == 1);

    gs_log_group_register(gs_can_log);

    uint32_t prescaler;
    gs_error_t res = calculate_prescaler(bps, &prescaler);
    if (res != GS_OK) {
        log_error("bit rate of %i not in supported range", bps);
        return res;
    }

    __HAL_RCC_CAN1_CLK_ENABLE();

    /* Exit from sleep mode */
    CAN1->MCR &= (~(uint32_t) CAN_MCR_SLEEP);

    /* Request initialization */
    CAN1->MCR |= CAN_MCR_INRQ;

    /* Wait the acknowledge */
    volatile uint32_t delay = 0;
    uint32_t clock_freq_hz = HAL_RCC_GetPCLK1Freq();
    while ((CAN1->MSR & CAN_MSR_INAK) != CAN_MSR_INAK) {
        delay++;
        if (delay >= clock_freq_hz) {
            log_error("CAN device did not enter initialization mode");
            return GS_ERROR_IO;
        }
    }

    CAN1->MCR &= ~(uint32_t) (CAN_MCR_TTCM | // Time Triggered Communication mode disabled
                              CAN_MCR_AWUM | // The Sleep mode is left on software request by clearing the SLEEP bit of the CAN_MCR register
                              CAN_MCR_NART | // The CAN hardware will automatically retransmit the message until it has been successfully transmitted according to the CAN standard
                              CAN_MCR_RFLM   // Receive FIFO not locked on overrun. Once a receive FIFO is full the next incoming message will overwrite the previous one
                              );

    /* Set the transmit FIFO priority */
    CAN1->MCR |= CAN_MCR_TXFP | // TX mailboxes is send chronologically
                 CAN_MCR_ABOM;  // The Bus-Off state is left automatically by hardware once 128 occurrences of 11 recessive bits have been monitored.

    /* Set the bit timing register */
    CAN1->BTR = (uint32_t) (CAN_BS1_4TQ |
                            CAN_BS2_5TQ |
                            (prescaler - 1));

    /* Request leave initialization */
    CAN1->MCR &= ~(uint32_t) CAN_MCR_INRQ;

    /* Wait for the acknowledge */
    delay = 0;
    while ((CAN1->MSR & CAN_MSR_INAK) == CAN_MSR_INAK) {
        delay++;
        if (delay >= clock_freq_hz) {
            log_error("CAN device did not leave initialization mode");
            return GS_ERROR_IO;
        }
    }

    res = gs_sem_create(1, &handle.tx_lock);
    if (res != GS_OK) {
        return res;
    }

    res = gs_sem_create(0, &handle.tx_sync_sem);
    if (res != GS_OK) {
        return res;
    }

    /* Enable TX interrupt */
    HAL_NVIC_SetPriority(CAN1_TX_IRQn, GS_STM32L4_DEFAULT_ISR_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);

    /* Enable RX FIFO 0 interrupt */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, GS_STM32L4_DEFAULT_ISR_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

    handle.inuse = true;

    return GS_OK;
}

gs_error_t gs_can_start(uint8_t device)
{
    GS_CHECK_ARG(device == 1);
    GS_CHECK_HANDLE(handle.inuse);

    /* Enable FIFO0 message pending and overrun interrupts */
    CAN1->IER |= CAN_IER_FOVIE0 | CAN_IER_FMPIE0;

    return GS_OK;
}

static inline gs_error_t gs_stm32l4_can_send(uint8_t device, uint32_t msg_id, const void * data, size_t data_size,
                                             int timeout_ms, bool extended)
{
    GS_CHECK_ARG(device == 1);
    GS_CHECK_ARG(data != NULL);
    GS_CHECK_ARG(data_size <= 8);

    gs_error_t res = gs_sem_wait(handle.tx_lock, timeout_ms);
    if (res != GS_OK) {
        log_error("did not get TX lock");
        return res;
    }

    /* Makes sure, that semaphore is taken */
    while (gs_sem_wait(handle.tx_sync_sem, 0) == GS_OK) {}

    /* Set up the Id
       Only the first TX mailbox out of three is used, as this send function will lock until completion */
    CAN1->sTxMailBox[0].TIR &= CAN_TI0R_TXRQ;
    if (extended) {
        CAN1->sTxMailBox[0].TIR |= (msg_id << 3) | CAN_ID_EXT | CAN_RTR_DATA;
    } else {
        CAN1->sTxMailBox[0].TIR |= (msg_id << 21) | CAN_RTR_DATA;
    }

    /* Set up the DLC */
    CAN1->sTxMailBox[0].TDTR &= (uint32_t) 0xFFFFFFF0;
    CAN1->sTxMailBox[0].TDTR |= data_size & 0x0000000F;

    /* Set up the data field */
    uint8_t * data8 = (uint8_t *) data;
    CAN1->sTxMailBox[0].TDLR = (((uint32_t) data8[3] << 24) |
                                ((uint32_t) data8[2] << 16) |
                                ((uint32_t) data8[1] << 8)  |
                                ((uint32_t) data8[0]));

    CAN1->sTxMailBox[0].TDHR = (((uint32_t) data8[7] << 24) |
                                ((uint32_t) data8[6] << 16) |
                                ((uint32_t) data8[5] << 8)  |
                                ((uint32_t) data8[4]));

    /* Clear completed flags */
    CAN1->TSR |= CAN_TSR_RQCP2 | CAN_TSR_RQCP1 | CAN_TSR_RQCP0;

    /* Enable TX mailbox empty interrupt */
    CAN1->IER |= CAN_IER_TMEIE;

    /* Request transmission */
    CAN1->sTxMailBox[0].TIR |= CAN_TI0R_TXRQ;

    res = gs_sem_wait(handle.tx_sync_sem, 100);
    if (res != GS_OK) {
        log_error("TX failed");
        /* Request abort */
        CAN1->TSR |= CAN_TSR_ABRQ0;
        res = gs_sem_wait(handle.tx_sync_sem, 100);
        if (res != GS_OK) {
            log_error("could not abort transmission");
        } else {
            res = GS_ERROR_IO;
        }
    }
    gs_sem_post(handle.tx_lock);

    return res;
}

gs_error_t gs_can_send_standard(uint8_t device, uint32_t msg_id, const void * data, size_t data_size, int timeout_ms)
{
    GS_CHECK_ARG(msg_id <= 0x7FF);
    return gs_stm32l4_can_send(device, msg_id, data, data_size, timeout_ms, false);
}

gs_error_t gs_can_send_extended(uint8_t device, uint32_t ext_msg_id, const void * data, size_t data_size, int timeout_ms)
{
    GS_CHECK_ARG(ext_msg_id <= 0x1FFFFFFF);
    return gs_stm32l4_can_send(device, ext_msg_id, data, data_size, timeout_ms, true);
}

static gs_error_t gs_stm32l4_can_set_filter_mask(uint8_t device, uint32_t msg_id, uint32_t mask, gs_can_rxdata_callback_t rx_callback, bool extended, void * rx_user_data)
{
    GS_CHECK_ARG(device == 1);
    GS_CHECK_ARG(rx_callback != NULL);

    if (handle.can_rx_callback != NULL) {
        /* Only one filter and one rx callback is supported as the driver is
         The STM however support multiple filters, so the driver can be updated in the future if needed */
        return GS_ERROR_IN_USE;
    }

     uint32_t filter_bit = ((uint32_t) 1) << 0;  // Filter 0

    /* Initialization mode for the filter */
    CAN1->FMR |= (uint32_t) CAN_FMR_FINIT;
    /* Deactivate */
    CAN1->FA1R &= ~(uint32_t) filter_bit;
    /* 32-bit scale for the filter */
    CAN1->FS1R |= filter_bit;
    /* 32-bit identifier or First 32-bit identifier */
    CAN1->sFilterRegister[0].FR1 = ((msg_id << 3) | (extended << 2)); // refer to fig 448 in MCU manual |
    /* 32-bit mask or Second 32-bit identifier */
    CAN1->sFilterRegister[0].FR2 = ((mask << 3) | (extended << 2));
    /*Id/Mask mode for the filter*/
    CAN1->FM1R &= ~(uint32_t) filter_bit;
    /* FIFO 0 assignation for the filter */
    CAN1->FFA1R &= ~(uint32_t) filter_bit;
    /* Activate filter */
    CAN1->FA1R |= filter_bit;
    /* Leave the initialization mode for the filter */
    CAN1->FMR &= ~((uint32_t) CAN_FMR_FINIT);

    handle.can_rx_callback = rx_callback;

    return GS_OK;
}

gs_error_t gs_can_set_standard_filter_mask(uint8_t device, uint32_t msg_id, uint32_t mask, gs_can_rxdata_callback_t rx_callback, void * rx_user_data)
{
    GS_CHECK_ARG(msg_id <= 0x7FF);
    return gs_stm32l4_can_set_filter_mask(device, msg_id, mask, rx_callback, false, rx_user_data);
}

gs_error_t gs_can_set_extended_filter_mask(uint8_t device, uint32_t ext_msg_id, uint32_t mask, gs_can_rxdata_callback_t rx_callback, void * rx_user_data)
{
    GS_CHECK_ARG(ext_msg_id <= 0x1FFFFFFF);
    return gs_stm32l4_can_set_filter_mask(device, ext_msg_id, mask, rx_callback, true, rx_user_data);
}

/* TX interrupt */
void CAN1_TX_IRQHandler(void)
{
    gs_context_switch_t ctx_switch = GS_CONTEXT_SWITCH_INIT;
    if ((CAN1->TSR & CAN_TSR_TME0) && (CAN1->IER & CAN_IER_TMEIE)) {
        /* Disable TX mailbox empty interrupt */
        CAN1->IER &= ~CAN_IER_TMEIE;
        gs_sem_post_isr(handle.tx_sync_sem, &ctx_switch);
    }
    portEND_SWITCHING_ISR(ctx_switch.task_woken);
}

/* RX FIFO 0 interrupt */
void CAN1_RX0_IRQHandler(void)
{
    gs_context_switch_t ctx_switch = GS_CONTEXT_SWITCH_INIT;

    /* Check Overrun flag for FIFO0 */
    if ((CAN1->RF0R & CAN_RF0R_FOVR0) && (CAN1->IER & CAN_IER_FOVIE0)) {
        /* Clear FIFO0 Overrun Flag */
        CAN1->RF0R &= ~CAN_RF0R_FOVR0;
        log_error_isr("CAN RX FIFO overflow");
    }

    /* Check if pending message */
    if ((CAN1->RF0R & CAN_RF0R_FMP0) && (CAN1->IER & CAN_IER_FMPIE0)) {

        /* Clear FIFO0 Message pending Flag */
        CAN1->RF0R &= ~CAN_RF0R_FMP0;

        /* Get the data field */
        handle.rx_buffer[0] = (uint8_t) 0xFF &  CAN1->sFIFOMailBox[0].RDLR;
        handle.rx_buffer[1] = (uint8_t) 0xFF & (CAN1->sFIFOMailBox[0].RDLR >> 8);
        handle.rx_buffer[2] = (uint8_t) 0xFF & (CAN1->sFIFOMailBox[0].RDLR >> 16);
        handle.rx_buffer[3] = (uint8_t) 0xFF & (CAN1->sFIFOMailBox[0].RDLR >> 24);
        handle.rx_buffer[4] = (uint8_t) 0xFF &  CAN1->sFIFOMailBox[0].RDHR;
        handle.rx_buffer[5] = (uint8_t) 0xFF & (CAN1->sFIFOMailBox[0].RDHR >> 8);
        handle.rx_buffer[6] = (uint8_t) 0xFF & (CAN1->sFIFOMailBox[0].RDHR >> 16);
        handle.rx_buffer[7] = (uint8_t) 0xFF & (CAN1->sFIFOMailBox[0].RDHR >> 24);

        uint32_t id = 0x1FFFFFFF & (CAN1->sFIFOMailBox[0].RIR >> 3);
        bool extended_id = CAN1->sFIFOMailBox[0].RIR & CAN_RI0R_IDE;
        size_t length = CAN1->sFIFOMailBox[0].RDTR & CAN_RDT0R_DLC;
        uint32_t time_now_ms = gs_time_rel_ms();

        /* Release the FIFO */
        CAN1->RF0R |= CAN_RF0R_RFOM0;

        handle.can_rx_callback(1, id, extended_id, handle.rx_buffer, length, time_now_ms, handle.rx_user_data, &ctx_switch);
    }
    portEND_SWITCHING_ISR(ctx_switch.task_woken);
}
