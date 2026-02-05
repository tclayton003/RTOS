#ifndef GS_A3200_PARAM_CONFIGURATION_H
#define GS_A3200_PARAM_CONFIGURATION_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions for \a configuration.
*/

#ifdef __cplusplus
extern "C" {
#endif

/** Table name */
#define GS_A3200_CONFIGURATION_TABLE_NAME                "configuration"
/** Table id */
#define GS_A3200_CONFIGURATION_TABLE_ID                  1
/** Legacy table id definition */
#define GS_A3200_CONFIGURATION_TABLE_MEM_ID              1
/** Table data size (bytes */
#define GS_A3200_CONFIGURATION_TABLE_SIZE                145 // bytes
/** Table rows (count) */
#define GS_A3200_CONFIGURATION_TABLE_ROWS                16

/**
   @defgroup param_addr Parameter addresses within the table.
   @{
*/
/** Address of gssb1_pwr_en: GSSB1 power */
#define GS_A3200_CONFIGURATION_GSSB1_PWR_EN                              0                  // 0x0000 bool
/** Address of gssb2_pwr_en: GSSB2 power */
#define GS_A3200_CONFIGURATION_GSSB2_PWR_EN                              1                  // 0x0001 bool
/** Address of flash_pwr_en: Flash power */
#define GS_A3200_CONFIGURATION_FLASH_PWR_EN                              2                  // 0x0002 bool
/** Address of pwm_pwr_en: PWM power */
#define GS_A3200_CONFIGURATION_PWM_PWR_EN                                3                  // 0x0003 bool
/** Address of swload_img1: Primary program image */
#define GS_A3200_CONFIGURATION_SWLOAD_IMG1                               4                  // 0x0004 string
/** Length/size of swload_img1 (bytes): Primary program image */
#define GS_A3200_CONFIGURATION_SWLOAD_IMG1_LENGTH                       32
/** Address of swload_cnt1: Primary program image boot count */
#define GS_A3200_CONFIGURATION_SWLOAD_CNT1                              36                  // 0x0024 uint16
/** Address of swload_img2: Secondary program image */
#define GS_A3200_CONFIGURATION_SWLOAD_IMG2                              38                  // 0x0026 string
/** Length/size of swload_img2 (bytes): Secondary program image */
#define GS_A3200_CONFIGURATION_SWLOAD_IMG2_LENGTH                       32
/** Address of swload_cnt2: Secondary program image boot count */
#define GS_A3200_CONFIGURATION_SWLOAD_CNT2                              70                  // 0x0046 uint16
/** Address of fs_default: Default partition */
#define GS_A3200_CONFIGURATION_FS_DEFAULT                               72                  // 0x0048 uint8
/** Address of fs_en_p0: Enable /flash0 */
#define GS_A3200_CONFIGURATION_FS_EN_P0                                 73                  // 0x0049 bool
/** Address of fs_en_p1: Enable /flash1 */
#define GS_A3200_CONFIGURATION_FS_EN_P1                                 74                  // 0x004a bool
/** Address of log_fram_en: Log to FRAM */
#define GS_A3200_CONFIGURATION_LOG_FRAM_EN                              75                  // 0x004b bool
/** Address of log_mask: Log mask/level settings */
#define GS_A3200_CONFIGURATION_LOG_MASK                                 76                  // 0x004c string
/** Length/size of log_mask (bytes): Log mask/level settings */
#define GS_A3200_CONFIGURATION_LOG_MASK_LENGTH                          64
/** Address of tsync_node: TSYNC CSP Addr */
#define GS_A3200_CONFIGURATION_TSYNC_NODE                              140                  // 0x008c uint8
/** Address of tsync_intv: TSYNC Interval */
#define GS_A3200_CONFIGURATION_TSYNC_INTV                              142                  // 0x008e uint16
/** Address of spi0_cs_mode: SPI0 CS Mode */
#define GS_A3200_CONFIGURATION_SPI0_CS_MODE                            144                  // 0x0090 uint8
/**@}*/
#ifdef __cplusplus
}
#endif
#endif
