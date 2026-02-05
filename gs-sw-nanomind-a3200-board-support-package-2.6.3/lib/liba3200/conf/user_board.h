#ifndef USER_BOARD_H_
#define USER_BOARD_H_

#include "compiler.h"
#include <conf_a3200.h>

#define FOSC32                      AVR32_SCIF_OSC32_FREQUENCY
#define OSC32_STARTUP               AVR32_SCIF_OSCCTRL32_STARTUP_8192_RCOSC
#define FOSC0                       16000000
#define OSC0_STARTUP                AVR32_SCIF_OSCCTRL0_STARTUP_2048_RCOSC

#define BOARD_OSC0_HZ               16000000
#define BOARD_OSC0_STARTUP_US       2000
#define BOARD_OSC0_IS_XTAL          true
#define BOARD_OSC32_HZ              32768
#define BOARD_OSC32_STARTUP_US      71000
#define BOARD_OSC32_IS_XTAL         false

#define SDRAM_PART_HDR              "mt48lc16m16a2tg7e/mt48lc16m16a2tg7e.h"
#define SDRAM_DBW                   16

#define LM71_SPI_CS                 2
#if (GS_A3200_BOARD_REVISION >= 3)
  #define LM71_SPI_CS_TEMP1_PIN     AVR32_PIN_PC00
#else
  #define LM71_SPI_CS_TEMP1_PIN     AVR32_PIN_PC01
#endif
#define LM71_SPI_CS_TEMP2_PIN       AVR32_PIN_PB23

#define FM33256_SPI_CS              3

#define SPN_FL512S_CS0              0
#define SPN_FL512S_CS1              1

#define PWR_GSSB_PIN                AVR32_PIN_PA28
#define PWR_SD_PIN                  AVR32_PIN_PA29
#define PWR_GSSB2_PIN               AVR32_PIN_PB25
#define PWR_PWM_PIN                 AVR32_PIN_PA21

#endif
