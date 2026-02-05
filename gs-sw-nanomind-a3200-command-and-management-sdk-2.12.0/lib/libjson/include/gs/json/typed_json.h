#ifndef LIBJSON_TYPED_JSON_H
#define LIBJSON_TYPED_JSON_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Helper functions for extracting values. These functions tries to validate the actual value, based on the requested type.
*/

#include <stdint.h>
#include <cjson/cJSON.h>
#include <gs/util/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Get number as \a int8.

   @param[in] json JSON object
   @param[in] label name of object
   @param[out] output value
   @return_gs_error_t
*/
gs_error_t gs_json_parse_int8(const cJSON* const json, const char* const label, int8_t* output);

/**
   Get number as \a uint8.

   @param[in] json JSON object
   @param[in] label name of object
   @param[out] output value
   @return_gs_error_t
*/
gs_error_t gs_json_parse_uint8(const cJSON* const json, const char* const label, uint8_t* output);

/**
   Get number as \a int16.

   @param[in] json JSON object
   @param[in] label name of object
   @param[out] output value
   @return_gs_error_t
*/
gs_error_t gs_json_parse_int16(const cJSON* const json, const char* const label, int16_t* output);

/**
   Get number as \a uint16.

   @param[in] json JSON object
   @param[in] label name of object
   @param[out] output value
   @return_gs_error_t
*/
gs_error_t gs_json_parse_uint16(const cJSON* const json, const char* const label, uint16_t* output);

/**
   Get number as \a int32.

   @param[in] json JSON object
   @param[in] label name of object
   @param[out] output value
   @return_gs_error_t
*/
gs_error_t gs_json_parse_int32(const cJSON* const json, const char* const label, int32_t* output);

/**
   Get number as \a uint32.

   @param[in] json JSON object
   @param[in] label name of object
   @param[out] output value
   @return_gs_error_t
*/
gs_error_t gs_json_parse_uint32(const cJSON* const json, const char* const label, uint32_t* output);

/**
   Get number as \a float.

   @param[in] json JSON object
   @param[in] label name of object
   @param[out] output value
   @return_gs_error_t
*/
gs_error_t gs_json_parse_float(const cJSON* const json, const char* const label, float* output);

/**
   Get number as \a double.

   @param[in] json JSON object
   @param[in] label name of object
   @param[out] output value
   @return_gs_error_t
*/
gs_error_t gs_json_parse_double(const cJSON* const json, const char* const label, double* output);

/**
   Get number as \a string.

   @param[in] json JSON object
   @param[in] label name of object
   @param[out] output value
   @return_gs_error_t
*/
gs_error_t gs_json_parse_string(const cJSON* const json, const char* const label, char** output);

/**
   Get number as \a bool.

   @param[in] json JSON object
   @param[in] label name of object
   @param[out] output value
   @return_gs_error_t
*/
gs_error_t gs_json_parse_bool(const cJSON* const json, const char* const label, uint8_t* output);

/**
   Get array.

   @param[in] json JSON object
   @param[in] label name of object
   @param[out] output array
   @param[out] output_size array size
   @return_gs_error_t
*/
gs_error_t gs_json_parse_array(const cJSON* const json, const char* const label, cJSON** output, int* output_size);

#ifdef __cplusplus
}
#endif
#endif
