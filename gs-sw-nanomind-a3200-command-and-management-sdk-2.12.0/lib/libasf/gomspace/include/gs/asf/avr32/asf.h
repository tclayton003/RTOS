#ifndef GS_ASF_AVR32_ASF_H
#define GS_ASF_AVR32_ASF_H

/**
   Convience include.
   Includes none self-contained headers.
   Includes original ASF header files with reduced warning level.
*/

#include <gs/util/types.h>

// not self contained
#include <avr32/io.h>
#include <wdt.h>

// shadow warnings
#pragma GCC diagnostic ignored "-Wshadow"
#include <sysclk.h>
#include <avr32_reset_cause.h>
#include <delay.h>
#pragma GCC diagnostic warning "-Wshadow"

#include <scif_uc3c.h>

#endif
