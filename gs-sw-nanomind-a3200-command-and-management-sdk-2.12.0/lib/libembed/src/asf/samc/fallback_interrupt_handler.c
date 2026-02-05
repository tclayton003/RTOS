/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/*
 * fallback_interrupt_handler.c
 */

#include <gs/util/types.h>
#include <gs/util/string.h>
#include <stdio.h>
#include <system_interrupt.h>

#define VECTOR(x)  case (x): return "" ASTRINGZ(x)

static const char * interrupt_vector_to_string(enum system_interrupt_vector vector)
{
    // no default, let compiler tell us what is missing
    switch (vector) {
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_NON_MASKABLE);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_HARD_FAULT);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_SV_CALL);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_PENDING_SV);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_SYSTICK);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_WDT);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_RTC);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_EIC);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_NVMCTRL);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_DMA);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_EVSYS);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_SERCOM0);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_SERCOM1);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_SERCOM2);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_SERCOM3);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_TCC0);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_TCC1);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_TCC2);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_TC0);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_TC1);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_TC2);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_TC3);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_TC4);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_ADC0);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_ADC1);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_SDADC);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_AC);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_DAC);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_PTC);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_CAN0);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_CAN1);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_SYSTEM);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_FREQM);
        // cppcheck-suppress CastIntegerToAddressAtReturn
        VECTOR(SYSTEM_INTERRUPT_MODULE_TSENS);
    }
    return "unknown vector";
}

void gs_samc_fallback_interrupt_handler(unsigned long stacked_pc)
{
    enum system_interrupt_vector vector = system_interrupt_get_active();
    printf("Unhandled interrupt: %d [%s], address: %p (enter while(1) loop)\r\n",
           vector,
           interrupt_vector_to_string(vector),
           GS_TYPES_INT2PTR(stacked_pc));

    for (;;);
}
