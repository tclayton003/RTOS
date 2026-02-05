/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
#define NOVATEL_PORT_AUTOSAMPLING   23
#define NOVATEL_PORT_BACKDOOR       24

#define PORT_COM1                   32
#define NOVA_PORT                   PORT_COM1

#define CMD_LOG                     1
#define CMD_FRESET                  20
#define CMD_RESET                   18
#define CMD_SERIALCONFIG            1246
#define CMD_SETPOS                  377
#define CMD_SETTIME                 102
#define CMD_ECUTOFF                 50
#define CMD_PPSCTRL                 613
#define CMD_SETTROPOMODEL           1434
#define CMD_SETPOSTIMEOUT           612
#define CMD_APPROXPOSTIMEOUT        1513
#define CMD_DOPPLERWINDOW           1512
#define CMD_FREQUENCYOUT            232
#define LOG_BESTPOS                 42
#define LOG_TRACKSTAT               83
#define LOG_RXSTATUS                93
#define LOG_TIME                    101
#define LOG_BESTXYZ                 241
#define LOG_HW                      963

#define TRIGGER_ONNEW               0
#define TRIGGER_ONCHANGED           1
#define TRIGGER_ONTIME              2
#define TRIGGER_ONNEXT              3
#define TRIGGER_ONCE                4
#define TRIGGER_ONMARK              5

#define MSG_TYPE_BIN                0
#define MSG_TYPE_ASCII              1
#define MSG_TYPE_ABB_ASCII          2

/* response id's */
#define RECEIVER_COM_ERR            -1
#define GPS_OK                      1
#define REQ_LOG_NOT_VALID           2
#define INVALID_MSG_ID              6
#define INVALID_CHECKSUM            8

#define NOVATEL_CSP_MAX_LEN         190

typedef struct __attribute__((__packed__)) novatel_csp_backdoor_s {
    uint8_t len;                // Number of bytes in data
    uint16_t timeout;            // Time in ms to wait for GPS response
    char data[NOVATEL_CSP_MAX_LEN];    // Used for holding ascii command for GPS
} novatel_csp_backdoor_t;

typedef struct __attribute__((__packed__)) novatel_csp_backdoor_res_s {
    uint8_t len;                // Number of bytes in data
    uint8_t data[NOVATEL_CSP_MAX_LEN];    // Used for holding ascii command for GPS
} novatel_csp_backdoor_res_t;


/* Novatel message header */
typedef struct __attribute__ ((packed)) novatel_msg_header_s {
    uint8_t sync[3];
    uint8_t header_lgth;
    uint16_t msg_id;
    uint8_t msg_type;
    uint8_t port_addr;
    uint16_t msg_lgth;
    uint16_t sequence;
    uint8_t idle_time;
    uint8_t time_status;
    uint16_t week;
    uint32_t ms;
    uint32_t receiver_status;
    uint16_t reserved;
    uint16_t receiver_sw_version;
} novatel_msg_header_t;

/* Novatel response header */
typedef struct __attribute__ ((packed)) novatel_response_header_s {
    novatel_msg_header_t msg;
} novatel_response_header_t;

/* Novatel log request command */
typedef struct __attribute__ ((packed)) novatel_log_request_s {
    uint32_t port;
    uint16_t msg_id;
    uint8_t msg_type;
    uint8_t reserved;
    uint32_t trigger;
    double period;
    double offset;
    uint32_t hold;
} novatel_log_request_t;

/* Novatel baudrate command */
typedef struct __attribute__ ((packed)) novatel_serial_config_s {
    uint32_t port;
    uint32_t baudrate;
    uint32_t parity;
    uint32_t databits;
    uint32_t stopbits;
    uint32_t handshake;
    uint32_t break_detection;
} novatel_serial_config_t;

/* Novatel reset command */
typedef struct novatel_reset_command_s {
    uint32_t target;
} novatel_reset_command_t;

typedef struct __attribute__ ((packed)) novatel_ppscontrol_s {
    uint32_t switch_var;
    uint32_t polarity;
    uint64_t period;
    uint32_t pulse_width;
} novatel_ppscontrol_t;

/* Novatel set approx time command*/
typedef struct __attribute__ ((packed)) novatel_approxtime_command_s {
    uint32_t week;
    uint64_t sec;
} novatel_approxtime_command_t;

/* Novatel set approx pos command */
typedef struct __attribute__ ((packed)) novatel_approxpos_command_s {
    uint64_t lat;
    uint64_t lon;
    uint64_t height;
} novatel_approxpos_command_t;

/* Novatel set elevation cutoff angle command */
typedef struct __attribute__ ((packed)) novatel_ecutoff_command_s {
    uint32_t angle;
} novatel_ecutoff_command_t;

/* Novatel set tropomodel */
typedef struct __attribute__ ((packed)) novatel_tropomodel_command_s {
    uint32_t model;
} novatel_tropomodel_command_t;

/* Novatel set postimeout */
typedef struct __attribute__ ((packed)) novatel_postimeout_command_s {
    uint32_t secs;
} novatel_postimeout_command_t;

/* Novatel set approxpostimeout */
typedef struct __attribute__ ((packed)) novatel_approxpostimeout_command_s {
    uint32_t secs;
} novatel_approxpostimeout_command_t;

/* Novatel set frequencyout */
typedef struct __attribute__ ((packed)) novatel_frequencyout_s {
    uint32_t switch_var;
    uint32_t pulse_width;
    uint32_t period;
} novatel_frequencyout_t;

/* Novatel set dopplerwindow */
typedef struct __attribute__ ((packed)) novatel_dopplerwindow_command_s {
    uint32_t system;
    uint32_t mode;
    uint32_t window;
    uint32_t centerfreq;
} novatel_dopplerwindow_command_t;
