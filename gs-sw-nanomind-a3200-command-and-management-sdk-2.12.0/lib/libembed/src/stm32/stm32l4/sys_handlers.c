/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <stm32l4xx_hal.h>
#include <stdio.h>
#include <gs/util/types.h>
#include <gs/embed/drivers/uart/uart.h>
#include <gs/embed/drivers/sys/reset.h>

void gs_fallback_interrupt_handler( uint32_t *pulFaultStackAddress )
{
    uint32_t program_counter; /* Program counter. */
    program_counter = pulFaultStackAddress[ 6 ];

    fprintf(stderr,"\n\rHardFault_handler:\n\rProgram counter address: %p\r\nResetting\n\r",
            GS_TYPES_INT2PTR(program_counter));

    RTC_HandleTypeDef handle;
    handle.Instance = RTC;
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&handle, RTC_BKP_DR30, program_counter);
    HAL_PWR_DisableBkUpAccess();

    gs_sys_reset(GS_SYS_RESET_EXCEPTION);
}

/* By default only the HardFault_Handler is enabled.
   Other faults, such as 'BusFault_Handler'and 'MemManage_Handler',
   defaults to the HardFault_Handler */
void HardFault_Handler(void)
{
    __asm volatile
        (
            " tst lr, #4                                                    \n"
            " ite eq                                                        \n"
            " mrseq r0, msp                                                 \n"
            " mrsne r0, psp                                                 \n"
            " ldr r1, [r0, #24]                                             \n"
            " ldr r2, handler2_address_const                                \n"
            " bx r2                                                         \n"
            " handler2_address_const: .word gs_fallback_interrupt_handler   \n"
        );
}

void vApplicationMallocFailedHook(void)
{
    fprintf(stderr, "\n\rvApplicationMallocFailedHook: NOMEM\r\n");
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char * pcTaskName)
{
    gs_sys_reset(GS_SYS_RESET_STACK_OVERFLOW);
}

void vApplicationIdleHook(void) {
    __WFI();
}
