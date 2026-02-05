/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/af_wheel/astro_wde.h>

#include <gs/embed/asf/drivers/spi/master.h>
#include <gs/util/log.h>
#include <string.h>
#include <math.h>

static GS_LOG_GROUP(gs_af_log, "astro_wde", GS_LOG_CAT_DRIVER, LOG_DEFAULT_MASK);
#define LOG_DEFAULT gs_af_log

/* SPI setup here */
#define ASTRO_WDE_CS		1	// 0 for a3200 header, 1 for motherboard
#define ASTRO_WDE_SPEED		400000
#define ASTRO_WDE_SPIMODE	0

#define 	AF_WCTRL		0x80 // Wheel Control register (dis/enable wheels)
#define 	AF_VEND_ACC_BASE	0x80 // Wheel speed and acc. register, base + wheel
#define 	AF_SOFT_RESET		0x8B // Restart bootloader
#define 	AF_FIRM_UP_ADD_DATA	0x8A // Firmware upload addr + data
#define 	AF_FIRM_UP_SEC		0x89 // Firmware security code

#define 	AF_TEL_RPM_BASE		0x00     // Telemetry MW speed base add
#define		AF_WDE_RPLY		0x04     // WDE reply message
#define 	AF_REBOOT_OK		0x010000
// Commands:
#define		AF_WHEEL_DIS		0x00    // Disable wheel
#define		AF_WHEEL_EN		0x01    // Enable wheel
// Masks:
#define 	AF_MASK_MW1		0	// MW1 mask
#define 	AF_MASK_MW2		1	// MW1 mask
#define 	AF_MASK_MW3		2	// MW1 mask
#define 	AF_MASK_MW4		3	// MW1 mask

// AF specs:
#define 	AF_TORQUE_QUANT		2.5		// Quantization steps for torque command
#define 	AF_SPEED_WRITE_QUANT	0.5	// Quantization steps for final speed precision
#define 	AF_SPEED_READ_QUANT	0.25	// Quantization steps for final speed precision
#define 	AF_RADS_MAX		1600	// Upper saturation (RAD/S)
#define 	AF_RADS_MIN		-1600	// Lower saturation (RAD/S)
#define		AF_RPM_MAX		16383	// Upper saturation (RPM)
#define		AF_RPM_MIN		-16384	// Lower saturation (RPM)
#define 	AF_RESTART_ACC		50	// Acceleration used after restart

#include <math.h>

static uint8_t astro_wde_spi_slave;

gs_error_t write_to_WDE(uint32_t data);
gs_error_t read_from_WDE(uint8_t address, uint32_t * data_out);

// 32 bit write package:
// <-- MSB -- -- LSB -->
// MOSI: <8-bit addr> <8-bit Write data 2> <8-bit Write data 1> <8-bit Write data 0>
// MISO: <8-bit don't care> <8-bit don't care> <8-bit don't care> <8-bit don't care>

// 32 bit read package:
// MOSI: <8-bit addr> <8-bit don't care> <8-bit don't care> <8-bit don't care>
// MISO: <8-bit don't care> <8-bit read data 2> <8-bit read data 1> <8-bit read data 0>

/**
 * Convert RADs to RPM
 * @param rads Radians
 * @return RPM
 */
static float rads2rpm(float rads)
{
    float rpm = rads * 60.0 / (2*M_PI);
    return rpm;
}

/**
 * Deadzone avoidance
 * @param speed Speed pointer
 */
static void deadzone(int16_t *speed)
{
    if (*speed > 0 && *speed < 60)
        *speed = 60;
    if (*speed < 0 && *speed > -60)
        *speed = -60;
}

/**
 * SPI Interface Initialization
 * @return GS Error Code
 */
gs_error_t af_wheels_init(uint8_t spi_slave)
{
    gs_log_group_register(LOG_DEFAULT);
    
    const gs_spi_chip_t spi_chip = {
        .handle = 0,
        .baudrate = ASTRO_WDE_SPEED,
        .bits = 8,
        .spi_mode = ASTRO_WDE_SPIMODE,
        .stay_act = 1,
        .trans_delay = 0,
        .chipselect = ASTRO_WDE_CS
    };
    astro_wde_spi_slave = spi_slave;
    return gs_spi_asf_master_configure_slave(astro_wde_spi_slave, &spi_chip);
}

#define SPI_TIMEOUT_MS 1000

/**
 * Function to read data from WDE
 * @param address Which address to access
 * @return 24 bit data (in uint32)
 */
gs_error_t read_from_WDE(uint8_t address, uint32_t * data_out)
{
    uint32_t read_data = 0;
    uint8_t data_tmp[3] = { 0 };
    address &= 0x00ff;

    gs_spi_master_trans_t transactions[] = {
         {
             .tx = &address,
             .rx = NULL,
             .size = sizeof(address),
         },
         {
             .tx = NULL,
             .rx = data_tmp,
             .size = 3,
         }
     };

    gs_error_t error = gs_spi_master_transactions(astro_wde_spi_slave, transactions,
                                                  GS_ARRAY_SIZE(transactions), SPI_TIMEOUT_MS);
    if(error)
    {
        return error;
    }

    memcpy(&read_data, data_tmp, 3);
    read_data = (read_data >> 8) & 0x00FFFFFF;

    *data_out = read_data;
    return GS_OK;
}

/**
 * Writes a 32 bit data package to the WDE
 * @param data Data to write to WDE
 * @return GS Error Code
 */
gs_error_t write_to_WDE(uint32_t data)
{
    uint8_t data_tmp[4] = { 0, 0, 0, 0 };

    for(int i = 0; i < 4; i++){
        data_tmp[i] = (data >> (8 * (3-i))) & 0xff;
    }

    gs_spi_master_trans_t transactions[] = {
         {
             .tx = data_tmp,
             .rx = NULL,
             .size = GS_ARRAY_SIZE(data_tmp)
         }
     };

    return gs_spi_master_transactions(astro_wde_spi_slave, transactions, GS_ARRAY_SIZE(transactions), SPI_TIMEOUT_MS);
}

/**
 * Enable/disable MW.
 * WDE address: AF_WCTRL
 * 32 bit write cmd: 8bit[Address] 20bit[don't care] 4bit[mw3 mw2 mw1 mw0], <- msb -- lsb ->
 */
gs_error_t af_set_enable(uint8_t wheel, uint8_t status, uint8_t *wheelPointer)
{

    uint32_t data = 0;
    data = (AF_WCTRL << 24);
    switch (wheel)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            wheelPointer[wheel] = status;
            break;
        case 5:
            for (uint8_t i=0; i<4; i++)
            {
                wheelPointer[i] = status;
            }
            break;
        default:
            return GS_ERROR_ARG;
    }

    // Set the enable bits for each wheel
    data += wheelPointer[0] ? 1 : 0;
    data += wheelPointer[1] ? 2 : 0;
    data += wheelPointer[2] ? 4 : 0;
    data += wheelPointer[3] ? 8 : 0;
    return write_to_WDE(data);
}

/**
 * Get the speed of a specific wheel
 * @param wheel Specific wheel (0-3)
 * @param speed Speed pointer
 */
gs_error_t af_get_speed(uint8_t wheel, float *speed)
{

    // Read speed from WDE
    int32_t speed_reading = 0;
    gs_error_t error = read_from_WDE((wheel) << AF_TEL_RPM_BASE, (uint32_t *)(&speed_reading));  // Raw speed reading in rpm
    if (error)
    {
        *speed = 0;
        return error;
    }

    // Make the data fit into a 32 bit signed int
    if (speed_reading & 0x00800000)
        speed_reading |= 0xFF800000;  // set upper 9 bits as 1
    else
        speed_reading &= 0x007fffff;  // set upper 9 bits as 0

    speed_reading = speed_reading * AF_SPEED_READ_QUANT;
    *speed = (float) speed_reading;

    return GS_OK;
}

/**
 * Command the wheel to spin up to a certain speed.
 * @param wheel Wheel number <0 - 3>
 * @param torque The desired delta torque in nNm
 * @param inertia Inertia of the wheel
 * @return GS Error code
 */
gs_error_t af_set_torque(uint8_t wheel, float torque, float inertia)
{
    float speed_rpm, final_speed, d_omega;
    uint32_t data = 0;
    int retval = GS_OK;

    if (torque == 0)
        return GS_OK;  // No need to continue

    if (wheel > 3) {  // Check if the param is within bounds
        return GS_ERROR_ARG;
    }

    /* Retrive speed and calc. delta speed */
    gs_error_t error = af_get_speed(wheel, &speed_rpm);
    if (error)
    {
        return error;
    }

    d_omega = 5 * torque / inertia;
    if (d_omega == NAN) {
        log_error("Devision by zero");
        return GS_ERROR_ARG;
    }
    d_omega = rads2rpm(d_omega);

    /* Calculate final speed */
    final_speed = d_omega + speed_rpm;
    if (final_speed > AF_RPM_MAX || final_speed < AF_RPM_MIN) {
        log_warning("Speed out of range, %f (tau=%f, I=%f)", d_omega, torque, inertia);
        return GS_ERROR_RANGE;
    }
    int16_t speed = (int16_t) round(final_speed / AF_SPEED_WRITE_QUANT);

    /* Avoid the deadzone around 0 */
    deadzone(&speed);

    /* Calculate acceleration */
    d_omega = (d_omega / AF_TORQUE_QUANT)/5;
    if (d_omega < -127 || d_omega > 127) {
        log_warning("Saturation");
        return GS_ERROR_ARG;
    }
    int8_t acc = (int8_t) round(d_omega);

    /* Write to WDE */
    data = ((((wheel + 1) | AF_VEND_ACC_BASE) << 24) & 0xff000000);  // Set address
    data += ((speed << 8) & 0xffff00);  // Set final speed
    data += acc & 0xff;  // Set acceleration
    retval = write_to_WDE(data);  // Write the data to WDE
    return retval;
}
