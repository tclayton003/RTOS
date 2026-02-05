#ifndef USER_BOARD_H_INCLUDED
#define USER_BOARD_H_INCLUDED

// DON'T include directly - use #include <board.h>

#include <clock.h> // libasf, GCLK.. constants
#include <port.h>  // libasf, GPIO Pin Control

// USART - used by libsamc/usart
/* #define GS_SAMC_USART_MODULE                 SERCOM1 */
/* #define GS_SAMC_UART_SERCOM_MUX_SETTING      USART_RX_1_TX_0_XCK_1 */
/* #define GS_SAMC_UART_SERCOM_PINMUX_PAD0      PINMUX_PA00D_SERCOM1_PAD0  // TX */
/* #define GS_SAMC_UART_SERCOM_PINMUX_PAD1      PINMUX_PA01D_SERCOM1_PAD1  // RX */
/* #define GS_SAMC_UART_SERCOM_PINMUX_PAD2      PINMUX_UNUSED */
/* #define GS_SAMC_UART_SERCOM_PINMUX_PAD3      PINMUX_UNUSED */

/* /\* I2C *\/ */
/* #define GSW_I2C_MODULE                       SERCOM3 */
/* #define GSW_I2C_SERCOM_PINMUX_PAD0           PINMUX_PA22C_SERCOM3_PAD0 // SDA */
/* #define GSW_I2C_SERCOM_PINMUX_PAD1           PINMUX_PA23C_SERCOM3_PAD1 // SCL */

/* // CAN enabled (input) or enable external CAN transceiver (output) */
/* #define GS_SAMC_CAN_GPIO                     PIN_PA19 */

/* // ADC instance - used by libsamc/adc.c */
/* #define GS_SAMC_ADC_INSTANCE                         ADC0 */

// ADC - common values
#define GS_ADC_CLOCK_SOURCE                          GCLK_GENERATOR_1
#define GS_ADC_CLOCK_PRESCALER                       ADC_CLOCK_PRESCALER_DIV4
#define GS_ADC_REF                                   ADC_REFERENCE_INTREF

/* // ADC - Current */
/* #define GS_ADC_VCURR_INPUT                           ADC_POSITIVE_INPUT_PIN1  // PA03 */

/* // ADC - DETI */
/* #define GS_ADC_DETI_INPUT                            ADC_POSITIVE_INPUT_PIN4  // PA04 */

/* // ADC - DETA */
/* #define GS_ADC_DETA_INPUT                            ADC_POSITIVE_INPUT_PIN5  // PA05 */

/* // ADC - DETB */
/* #define GS_ADC_DETB_INPUT                            ADC_POSITIVE_INPUT_PIN6  // PA06 */

/* // ADC - External EC20 temperature - used by libsamc/tsense_external.c */
#define GS_SAMC_TSENSE_EXTERNAL_ADC_CLOCK_SOURCE     GS_ADC_CLOCK_SOURCE
#define GS_SAMC_TSENSE_EXTERNAL_ADC_CLOCK_PRESCALER  GS_ADC_CLOCK_PRESCALER
#define GS_SAMC_TSENSE_EXTERNAL_ADC_INPUT            ADC_POSITIVE_INPUT_PIN7  // PA07
#define GS_SAMC_TSENSE_EXTERNAL_ADC_REF              GS_ADC_REF;
#define GS_SAMC_TSENSE_EXTERNAL_ADC_VREF             (2.048F) // = internal voltage reference

/* // DAC - VPACON - always on PA02 */
/* #define GS_DAC_CLOCK_SOURCE                          GCLK_GENERATOR_0 */
/* // 10 bit DAC */
/* #define GS_DAC_MAX_VALUE						     1023 */


/* // Debug */
/* // PA30 - SWCLK */
/* // PA31 - SWDIO */

/* // GPO */
/* #define GS_OUTPUT_PAON_A            PIN_PA08 */
/* #define GS_OUTPUT_PAON_B            PIN_PA09 */
/* #define GS_OUTPUT_TXON              PIN_PA10 */
/* #define GS_OUTPUT_RXON              PIN_PA11 */
/* #define GS_OUTPUT_TXEN              PIN_PA14 */
/* #define GS_OUTPUT_PAEN_A            PIN_PA15 */
/* #define GS_OUTPUT_PAEN_B            PIN_PA16 */

/* // GPI */
/* #define GS_INPUT_TXON_EX            PIN_PA17 */
/* #define GS_INPUT_TXON_EX_EIC_CHAN   1 */
/* #define GS_INPUT_TXON_EX_EIC_PIN    PIN_PA17A_EIC_EXTINT1 */
/* #define GS_INPUT_TXON_EX_EIC_MUX    MUX_PA17A_EIC_EXTINT1 */

/* #define GS_INPUT_RXON_EX            PIN_PA18 */
/* #define GS_INPUT_RXON_EX_EIC_CHAN   2 */
/* #define GS_INPUT_RXON_EX_EIC_PIN    PIN_PA18A_EIC_EXTINT2 */
/* #define GS_INPUT_RXON_EX_EIC_MUX    MUX_PA18A_EIC_EXTINT2 */

/* #define GS_INPUT_TXEN_EX            PIN_PA27 */
/* #define GS_INPUT_TXEN_EX_EIC_CHAN   15 */
/* #define GS_INPUT_TXEN_EX_EIC_PIN    PIN_PA27A_EIC_EXTINT15 */
/* #define GS_INPUT_TXEN_EX_EIC_MUX    MUX_PA27A_EIC_EXTINT15 */

/* #define GS_INPUT_PIN_OK             PIN_PA28 */
/* #define GS_INPUT_PIN_OK_EIC_CHAN    8 */
/* #define GS_INPUT_PIN_OK_EIC_PIN     PIN_PA28A_EIC_EXTINT8 */
/* #define GS_INPUT_PIN_OK_EIC_MUX     MUX_PA28A_EIC_EXTINT8 */

#endif
