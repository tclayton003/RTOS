#ifndef GS_THIRDPARTY_NOVATEL_GNSS_NOVATEL_GNSS_H
#define GS_THIRDPARTY_NOVATEL_GNSS_NOVATEL_GNSS_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <stdint.h>
#include <gs/util/error.h>

/* Novatel bestpos log response */
typedef struct __attribute__ ((packed)) novatel_bestpos_s {
    uint32_t sol_stat;
    uint32_t pos_type;
    double lat;
    double lon;
    double hgt;
    float undulation;
    uint32_t datum_id;
    float lat_deviation;
    float lon_deviation;
    float hgt_deviation;
    char stn_id[4];
    float diff_age;
    float sol_age;
    uint8_t SVs;
    uint8_t solnSVs;
    uint8_t solnL1SVs;
    uint8_t solnMultiSVs;
    uint8_t reserved;
    uint8_t ext_sol_stat;
    uint8_t galileo_sig_mask;
    uint8_t GPS_GLONASS_sig_mask;
    uint32_t crc;
} novatel_bestpos_t;

/* Novatel bestxyz log response */
typedef struct __attribute__ ((packed)) novatel_bestxyz_s {
    uint32_t pos_sol_stat;
    uint32_t pos_type;
    uint64_t px;
    uint64_t py;
    uint64_t pz;
    uint32_t px_deviation;
    uint32_t py_deviation;
    uint32_t pz_deviation;
    uint32_t vel_sol_stat;
    uint32_t vel_type;
    uint64_t vx;
    uint64_t vy;
    uint64_t vz;
    uint32_t vx_deviation;
    uint32_t vy_deviation;
    uint32_t vz_deviation;
    char stn_id[4];
    float v_latency;
    float diff_age;
    float sol_age;
    uint8_t SVs;
    uint8_t solnSVs;
    uint8_t ggL1;
    uint8_t solnMultiSVs;
    uint8_t reserved;
    uint8_t ext_sol_stat;
    uint8_t galileo_sig_mask;
    uint8_t GPS_GLONASS_sig_mask;
    uint32_t crc;
} novatel_bestxyz_t;

typedef struct __attribute__ ((packed)) novatel_trackstat_sat_s {
    uint16_t prn;
    uint16_t glofreq;
    uint32_t ch_tr_status;
    uint64_t psr;
    uint32_t doppler;
    uint32_t c_no;
    uint32_t locktime;
    uint32_t psr_res;
    uint32_t reject;
    uint32_t psr_weigth;
} novatel_trackstat_sat_t;

typedef struct __attribute__ ((packed)) novatel_trackstat_s {
    uint32_t sol_status;
    uint32_t pos_type;
    uint32_t cutoff;
    uint32_t num_of_chans;
    novatel_trackstat_sat_t sat[20];
} novatel_trackstat_t;

typedef struct __attribute__ ((packed)) novatel_hwstatus_s {
    uint32_t numofmeas;
    uint32_t value;
    uint8_t status;
    uint8_t meas;
    uint16_t dummy;
} novatel_hwstatus_t;


/* Novatel rxstatus log response */
typedef struct __attribute__ ((packed)) novatel_rxstatus_s {
    uint32_t error;
    uint32_t nr_stats;
    uint32_t rxstat;
    uint32_t rxstat_set;
    uint32_t rxstat_clear;
    uint32_t aux1stat;
    uint32_t aux1stat_pri;
    uint32_t aux1stat_set;
    uint32_t aux1stat_clear;
    uint32_t aux2stat;
    uint32_t aux2stat_pri;
    uint32_t aux2stat_set;
    uint32_t aux2stat_clear;
    uint32_t aux3stat;
    uint32_t aux3stat_pri;
    uint32_t aux3stat_set;
    uint32_t aux3stat_clear;
    uint32_t crc;
} novatel_rxstatus_t;

/* Novatel time log response */
typedef struct __attribute__ ((packed)) novatel_time_s {
    uint32_t    clock_status;
    double        offset;
    double        offset_std;
    double        utc_offset;
    uint32_t    utc_year;
    uint8_t     utc_month;
    uint8_t     utc_day;
    uint8_t     utc_hour;
    uint8_t     utc_min;
    uint32_t     utc_ms;
    uint32_t     utc_status;
} novatel_time_t;



typedef enum {
    TROPO_MODEL_NONE = 1,
    TROPO_MODEL_AUTO = 2,
} novatel_tropo_model_t;

typedef enum {
    ASSIGN_SYSTEM_ALL = 3,
    ASSIGN_SYSTEM_GPS = 99,
    ASSIGN_SYSTEM_SBAS = 100,
    ASSIGN_SYSTEM_GLONASS = 101,
    ASSIGN_SYSTEM_GALILEA = 102,
    ASSIGN_SYSTEM_BEIDOU = 103,
    ASSIGN_SYSTEM_QZSS = 104,
    ASSIGN_SYSTEM_NAVIC = 105,
} novatel_assign_system_t;

typedef enum {
    DOPPLER_MODE_AUTO = 0,
    DOPPLER_MODE_USER = 1,
} novatel_doppler_mode_t;

void novatel_print_bestpos(novatel_bestpos_t * bestpos);
void novatel_print_bestxyz(novatel_bestxyz_t * msg);
gs_error_t novatel_get_bestpos(novatel_bestpos_t * bestpos);
gs_error_t novatel_get_bestxyz(novatel_bestxyz_t * bestxyz);
gs_error_t novatel_get_receiver_status(novatel_rxstatus_t * rxstatus);
gs_error_t novatel_get_receiver_time(novatel_time_t * time);
gs_error_t novatel_get_temperature(novatel_hwstatus_t * temp);
gs_error_t novatel_get_trackstat(novatel_trackstat_t * trackstat);
gs_error_t novatel_set_serial_baudrate(uint32_t baudrate, uint8_t parity, uint8_t databits, uint8_t stopbits);
gs_error_t novatel_reset_receiver(void);
gs_error_t novatel_set_approxpos(uint64_t lat, uint64_t lon, uint64_t height);
gs_error_t novatel_set_approxtime(uint32_t week, uint64_t sec);
gs_error_t novatel_set_ecutoff(float angle);
gs_error_t novatel_set_ppscontrol(void);
gs_error_t novatel_set_tropomodel(novatel_tropo_model_t model);
gs_error_t novatel_set_postimeout(uint32_t secs);
gs_error_t novatel_set_dopplerwindow(novatel_assign_system_t assign_system, novatel_doppler_mode_t mode, uint32_t window_hz);
gs_error_t novatel_set_approxpos_timeout(uint32_t secs);
gs_error_t novatel_set_frequencyout(uint8_t en_varf);

/**
 * ascii_command is used for ascii communication with the gps.
 *
 * @param cmd pointer to ascii command for GPS
 * @param cmd_len length of command pointed to by cmd
 * @param response pointer to buffer to store the command response from the GPS
 * @param max_res_len maximum length of GPS response
 * @param timeout_ms time in ms to wait for the response from the GPS
 * @return length of response from GPS
 */
int novatel_ascii_command(const char * cmd,
                          uint16_t cmd_len,
                          unsigned char * response,
                          uint16_t max_res_len,
                          uint16_t timeout_ms);

#endif
