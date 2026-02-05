/** NOTE:
 * Remember to run 'waf clean' whenever any value in this file is changed.
 * This is because 'waf' does not rebuild the libraries which use this header file
 * because of an issue in the dependency system.
 */

#ifndef CONF_CLOCK_H_INCLUDED
#define CONF_CLOCK_H_INCLUDED

/* Define which clocks to enable */
#define CONFIG_SYSCLK_INIT_CPUMASK  0x0
//#define CONFIG_SYSCLK_INIT_PBAMASK  0xF8225ABF
//#define CONFIG_SYSCLK_INIT_PBBMASK  0xFFFFFFBD
//#define CONFIG_SYSCLK_INIT_PBCMASK  0xFFFFFD7D
//#define CONFIG_SYSCLK_INIT_HSBMASK  0xFFFFF7E7


//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_RCSYS
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_OSC0
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_OSC1
#define CONFIG_SYSCLK_SOURCE          SYSCLK_SRC_PLL0
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_PLL1
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_RC8M

/* Fbus = Fsys / (2 ^ BUS_div) */
#define CONFIG_SYSCLK_CPU_DIV         0
#define CONFIG_SYSCLK_PBA_DIV         0
#define CONFIG_SYSCLK_PBB_DIV         0
#define CONFIG_SYSCLK_PBC_DIV         0

#define CONFIG_PLL0_SOURCE            PLL_SRC_OSC0
//#define CONFIG_PLL0_SOURCE          PLL_SRC_OSC1
//#define CONFIG_PLL0_SOURCE          PLL_SRC_RC8M

/* Fpll0 = (Fclk * PLL_mul) / PLL_div */
#define CONFIG_PLL0_MUL               2
#define CONFIG_PLL0_DIV               1

#endif /* CONF_CLOCK_H_INCLUDED */
