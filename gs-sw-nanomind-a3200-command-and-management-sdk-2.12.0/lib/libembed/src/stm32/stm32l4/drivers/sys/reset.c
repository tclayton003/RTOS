/* Copyright (c) 2013-201 GomSpace A/S. All rights reserved. */

#include <stm32l4xx_hal.h>
#include <gs/embed/drivers/sys/reset.h>

void gs_sys_reset(gs_sys_reset_cause_t reset_cause)
{
    RTC_HandleTypeDef handle;
    handle.Instance = RTC;

    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&handle, RTC_BKP_DR31, reset_cause);
    HAL_PWR_DisableBkUpAccess();

    __HAL_RCC_CLEAR_RESET_FLAGS();

    NVIC_SystemReset();
}

static uint32_t current_reset_cause = UINT32_MAX;

gs_sys_reset_cause_t gs_sys_get_reset_cause(void)
{
    if (current_reset_cause == UINT32_MAX) {
        RTC_HandleTypeDef handle;
        handle.Instance = RTC;
        current_reset_cause = HAL_RTCEx_BKUPRead(&handle, RTC_BKP_DR31);
        __HAL_RCC_PWR_CLK_ENABLE();
        HAL_PWR_EnableBkUpAccess();
        HAL_RTCEx_BKUPWrite(&handle, RTC_BKP_DR31, GS_SYS_RESET_UNKNOWN);
        HAL_PWR_DisableBkUpAccess();
    }
    return current_reset_cause;
}

gs_sys_boot_cause_t gs_sys_get_boot_cause(uint32_t * platform_boot_cause)
{
    gs_sys_boot_cause_t generic_boot_cause = GS_SYS_BOOT_UNKNOWN;
    if(__HAL_RCC_GET_FLAG(RCC_FLAG_FWRST)) {
        generic_boot_cause = GS_SYS_BOOT_ERROR;
    } else if(__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)) {
        generic_boot_cause =  GS_SYS_BOOT_ERROR;
    } else if(__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST)) {
        generic_boot_cause =  GS_SYS_BOOT_ERROR;
    } else if(__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST)) {
        generic_boot_cause =  GS_SYS_BOOT_BROWN_OUT;
    } else if(__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) {
        generic_boot_cause =  GS_SYS_BOOT_SW;
    } else if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
        generic_boot_cause =  GS_SYS_BOOT_WATCHDOG;
    } else if(__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)) {
        generic_boot_cause =  GS_SYS_BOOT_WATCHDOG;
    } else if(__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) {
        generic_boot_cause =  GS_SYS_BOOT_PIN_EXTERNAL;
    }

    if (platform_boot_cause) {
        *platform_boot_cause = RCC->CSR & 0xf000;
    }
    __HAL_RCC_CLEAR_RESET_FLAGS();

    return generic_boot_cause;
}

static uint32_t current_crash_address = UINT32_MAX;

uint32_t gs_sys_get_crash_address()
{
    if (current_crash_address == UINT32_MAX) {
        RTC_HandleTypeDef handle;
        handle.Instance = RTC;
        current_crash_address = HAL_RTCEx_BKUPRead(&handle, RTC_BKP_DR30);
        if (current_crash_address == 0) {
            current_crash_address = UINT32_MAX;
        }
        __HAL_RCC_PWR_CLK_ENABLE();
        HAL_PWR_EnableBkUpAccess();
        HAL_RTCEx_BKUPWrite(&handle, RTC_BKP_DR30, UINT32_MAX);
        HAL_PWR_DisableBkUpAccess();
    }

    return current_crash_address;
}

void gs_sys_clear_reset_cause(void)
{
    /* Not implemented - it is not needed on STM32 platforms until we need Flash->RAM boot scenario */
    /* This function is currently only used on AVR32 for reference. */
}
