/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/embed/asf/delay.h>

#if defined(__ARMEL__) && !defined(__THUMBEL__)
#define GS_UTIL_DELAY_IMPL 1

/* Measured value. This will change if you update the wait states */
static const uint32_t cycles_per_loop = 30;
static inline void __attribute__ ((always_inline)) delay_cycles(uint32_t cycles) {
    asm volatile (
        "L_%=:					\n\t"
        "	nop					\n\t" 
        "	subs %[c], %[c], #1	\n\t"
        "	bne L_%=			\n\t"
        : /* No output */
        : [c] "r" (cycles)
        : /* No clobbered registers */
                  );
}

#elif defined(__AVR__)
#define GS_UTIL_DELAY_IMPL 1

#include <util/delay_basic.h>
/* http://www.nongnu.org/avr-libc/user-manual/group__util__delay__basic.html */
static const uint32_t cycles_per_loop = 4;
static inline void __attribute__ ((always_inline)) delay_cycles(uint32_t cycles) {
    _delay_loop_2((uint16_t) cycles);
}

#elif defined(__AVR32__)
#define GS_UTIL_DELAY_IMPL 1

/* NOTE: This value has not been measured */
static const uint32_t cycles_per_loop = 6;
static inline void __attribute__ ((always_inline)) delay_cycles(uint32_t cycles) {
    __asm__ (
        "L_%=:				\n\t"
        "	nop				\n\t" 
        "	sub %[c], 1		\n\t"
        "	brne L_%=		\n\t"
        : /* No output */
        : [c] "r" (cycles)
             );
}
#endif

#if GS_UTIL_DELAY_IMPL
/* Init to 8 MHz to at least get some delay if delay_init is not called */
static uint32_t fcpu = 40000000UL;
static const uint32_t us_per_s = 1000000UL;

void gs_delay_us(uint32_t us)
{
    /* Round up so we loop at least once */
    uint32_t cycles = (uint32_t)((((uint64_t)us * (uint64_t)fcpu) / us_per_s) / cycles_per_loop + 1);
    delay_cycles(cycles);
}

gs_error_t gs_delay_init(uint32_t freq)
{
    fcpu = freq;
    return GS_OK;
}

#endif // GS_UTIL_DELAY_IMPL
