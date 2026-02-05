#ifndef GS_A3200_SPI_SLAVE_H
#define GS_A3200_SPI_SLAVE_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Definition of logical SPI slave ids on A3200.

   The \a bus and \a CS (chip-select) are typical values, but can be changed without affecting the slave id.
*/

#ifdef __cplusplus
extern "C" {
#endif

#define GS_A3200_SPI_SLAVE_ADCS_WHEEL_0    0 //!< Reaction wheel ->  bus: 0, CS: 0 (decoded)
#define GS_A3200_SPI_SLAVE_ADCS_WHEEL_1    1 //!< Reaction wheel ->  bus: 0, CS: 1 (decoded)
#define GS_A3200_SPI_SLAVE_ADCS_WHEEL_2    2 //!< Reaction wheel ->  bus: 0, CS: 2 (decoded)
#define GS_A3200_SPI_SLAVE_ADCS_WHEEL_3    3 //!< Reaction wheel ->  bus: 0, CS: 3 (decoded)
#define GS_A3200_SPI_SLAVE_FRAM            4 //!< FRAM           ->  bus: 1, CS: 3
#define GS_A3200_SPI_SLAVE_LM71_0          5 //!< Temperature    ->  bus: 1, CS: 2 (gpio)
#define GS_A3200_SPI_SLAVE_SPN_FL512_0     6 //!< FLASH part 0   ->  bus: 1, CS: 0
#define GS_A3200_SPI_SLAVE_SPN_FL512_1     7 //!< FLASH part 1   ->  bus: 1, CS: 1
#define GS_A3200_SPI_SLAVE_LM71_1          8 //!< Temperature    ->  bus: 1, CS: 2 (gpio)
#define GS_A3200_SPI_SLAVE_ADCS_EXT_GYRO   9 //!< Ext. Gyro      ->  bus: 0, CS: 4 (decoded)

#ifdef __cplusplus
}
#endif
#endif
