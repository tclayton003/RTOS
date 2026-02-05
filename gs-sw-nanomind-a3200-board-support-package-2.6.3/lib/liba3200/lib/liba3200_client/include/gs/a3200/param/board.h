#ifndef GS_A3200_PARAM_BOARD_H
#define GS_A3200_PARAM_BOARD_H
/* Copyright (c) 2013 GomSpace A/S. All rights reserved. (AUTO GENERATED) */
/**
   @file
   Table definitions for \a board.
*/

#ifdef __cplusplus
extern "C" {
#endif

/** Table name */
#define GS_A3200_BOARD_TABLE_NAME                        "board"
/** Table id */
#define GS_A3200_BOARD_TABLE_ID                          0
/** Legacy table id definition */
#define GS_A3200_BOARD_TABLE_MEM_ID                      0
/** Table data size (bytes */
#define GS_A3200_BOARD_TABLE_SIZE                        157 // bytes
/** Table rows (count) */
#define GS_A3200_BOARD_TABLE_ROWS                        15

/**
   @defgroup param_addr Parameter addresses within the table.
   @{
*/
/** Address of uid: Unit IDentification */
#define GS_A3200_BOARD_UID                                               0                  // 0x0000 string
/** Length/size of uid (bytes): Unit IDentification */
#define GS_A3200_BOARD_UID_LENGTH                                       16
/** Address of type: Type */
#define GS_A3200_BOARD_TYPE                                             16                  // 0x0010 uint8
/** Address of rev: Revision */
#define GS_A3200_BOARD_REV                                              17                  // 0x0011 uint8
/** Address of dock_type: Dock type */
#define GS_A3200_BOARD_DOCK_TYPE                                        18                  // 0x0012 string
/** Length/size of dock_type (bytes): Dock type */
#define GS_A3200_BOARD_DOCK_TYPE_LENGTH                                 16
/** Address of csp_addr: CSP address */
#define GS_A3200_BOARD_CSP_ADDR                                         34                  // 0x0022 uint8
/** Address of csp_rtable: CSP routing table */
#define GS_A3200_BOARD_CSP_RTABLE                                       35                  // 0x0023 string
/** Length/size of csp_rtable (bytes): CSP routing table */
#define GS_A3200_BOARD_CSP_RTABLE_LENGTH                                96
/** Address of gosh_en: GOSH enabled */
#define GS_A3200_BOARD_GOSH_EN                                         131                  // 0x0083 bool
/** Address of gosh_uart: GOSH UART */
#define GS_A3200_BOARD_GOSH_UART                                       132                  // 0x0084 uint8
/** Address of can_en: CAN enabled */
#define GS_A3200_BOARD_CAN_EN                                          133                  // 0x0085 bool
/** Address of can_brate: CAN bitrate */
#define GS_A3200_BOARD_CAN_BRATE                                       136                  // 0x0088 uint32
/** Address of i2c_en: I2C enabled */
#define GS_A3200_BOARD_I2C_EN                                          140                  // 0x008c bool
/** Address of i2c_brate: I2C bitrate */
#define GS_A3200_BOARD_I2C_BRATE                                       144                  // 0x0090 uint32
/** Address of kiss_en: KISS enabled */
#define GS_A3200_BOARD_KISS_EN                                         148                  // 0x0094 bool
/** Address of kiss_brate: KISS baudrate */
#define GS_A3200_BOARD_KISS_BRATE                                      152                  // 0x0098 uint32
/** Address of kiss_uart: KISS UART */
#define GS_A3200_BOARD_KISS_UART                                       156                  // 0x009c uint8
/**@}*/
#ifdef __cplusplus
}
#endif
#endif
