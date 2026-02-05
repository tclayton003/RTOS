/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
#include <gs/util/error.h>

/**
  Bit 0 represents a general/unknown error
*/
#define BST_ST_DEF_STATUS_REG_ERROR (1<<0)
/**
  Bit 1 reports a successful acquisition
*/
#define BST_ST_DEF_STATUS_REG_DUMP_SUCC (1<<1)

/**
  @file

  Driver for Hyperion ST200 star tracker
*/

/**
  Driver handle
*/
typedef struct {
    /**
      Uart device id
    */
    uint8_t device;
} gs_hyp_st200_handle_t;


/**
  Standard frame received from the ST200
*/
typedef struct {
    /**
      Frame header
    */
    uint8_t header;
    /**
      Device ID
    */
    uint8_t dev_id;
    /**
      Status register
    */
    uint8_t status_register;
    /**
      Exposure time
    */
    uint16_t exposure_time;
    /**
      Time in seconds
    */
    uint32_t time_tag_s;
    /**
      Time sub-seconds part
    */
    uint16_t time_tag_sub_s;
    /**
      Q1
    */
    float q1;
    /**
      Q2
    */
    float q2;
    /**
      Q3
    */
    float q3;
    /**
      Q4
    */
    float q4;
    /**
      Pitch
    */
    float pitch;
    /**
      Yaw
    */
    float yaw;
    /**
      Roll
    */
    float roll;
    /**
      Number of identified stars
    */
    int8_t stars_ident;
    /**
      CRC value from frame
    */
    uint8_t crc;
} gs_hyp_st200_standard_frame_t;

/**
  Initialize the sensor

  @param device uart
  @param handle handle used for communication
  @return GS_OK on success
*/
gs_error_t gs_hyp_st200_init(const uint8_t device, gs_hyp_st200_handle_t * handle);

/**
  Get sample from sensor

  @param handle handle used for communication
  @param sample sample containing the reply from the sensor
  @return GS_OK on success
*/
gs_error_t gs_hyp_st200_sample(const gs_hyp_st200_handle_t handle, gs_hyp_st200_standard_frame_t *sample);

/**
  Send a command to the sensor

  @param handle handle used for communication
  @param command the ascii command to send to the sensor
  @param no_response set to true if no response is exptected from the sensor
  @param print_response set to true if response is to be printed
  @return GS_OK on success
*/
gs_error_t gs_hyp_st200_send_command(const gs_hyp_st200_handle_t handle, const char* command, bool no_response, bool print_response);

/**
  Print a frame to stdout

  @param frame received frame from the sensor
*/
void gs_hyp_st200_show_frame(const gs_hyp_st200_standard_frame_t frame);

/**
  Run CRC on input frame

  @param input the destuffed input frame
  @param input_length length of input array
  @return GS_OK on success
*/
gs_error_t gs_hyp_st200_crc(uint8_t *input, size_t input_length);

/**
  Initialize the GOSH commands for the sensor
*/
void gs_hyp_st200_register_commands(void);
