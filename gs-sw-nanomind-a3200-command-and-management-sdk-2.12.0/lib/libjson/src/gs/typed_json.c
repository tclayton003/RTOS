/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include "local.h"

gs_error_t gs_json_parse_int8(const cJSON* const json, const char* const label, int8_t* output)
{
    const cJSON* json_output = cJSON_GetObjectItemCaseSensitive(json, label);
    
    if (json_output == NULL) return GS_ERROR_NOT_FOUND;
    if (!cJSON_IsNumber(json_output)) return GS_ERROR_TYPE;
    if (json_output->valueint > INT8_MAX) return GS_ERROR_OVERFLOW;
    if (json_output->valueint < INT8_MIN) return GS_ERROR_OVERFLOW;

    *output = json_output->valueint;

    return GS_OK;
}

gs_error_t gs_json_parse_uint8(const cJSON* const json, const char* const label, uint8_t* output)
{
    const cJSON* json_output = cJSON_GetObjectItemCaseSensitive(json, label);
    
    if (json_output == NULL) return GS_ERROR_NOT_FOUND;
    if (!cJSON_IsNumber(json_output)) return GS_ERROR_TYPE;
    if (json_output->valueint > UINT8_MAX) return GS_ERROR_OVERFLOW;
    if (json_output->valueint < 0) return GS_ERROR_OVERFLOW;

    *output = json_output->valueint;
    
    return GS_OK;
}

gs_error_t gs_json_parse_int16(const cJSON* const json, const char* const label, int16_t* output)
{
    const cJSON* json_output = cJSON_GetObjectItemCaseSensitive(json, label);
    
    if (json_output == NULL) return GS_ERROR_NOT_FOUND;
    if (!cJSON_IsNumber(json_output)) return GS_ERROR_TYPE;
    if (json_output->valueint > INT16_MAX) return GS_ERROR_OVERFLOW;
    if (json_output->valueint < INT16_MIN) return GS_ERROR_OVERFLOW;

    *output = json_output->valueint;

    return GS_OK;
}

gs_error_t gs_json_parse_uint16(const cJSON* const json, const char* const label, uint16_t* output)
{
    const cJSON* json_output = cJSON_GetObjectItemCaseSensitive(json, label);
    
    if (json_output == NULL) return GS_ERROR_NOT_FOUND;
    if (!cJSON_IsNumber(json_output)) return GS_ERROR_TYPE;
    if (json_output->valueint > UINT16_MAX) return GS_ERROR_OVERFLOW;
    if (json_output->valueint < 0) return GS_ERROR_OVERFLOW;

    *output = json_output->valueint;

    return GS_OK;
}

gs_error_t gs_json_parse_int32(const cJSON* const json, const char* const label, int32_t* output)
{
    const cJSON* json_output = cJSON_GetObjectItemCaseSensitive(json, label);
    
    if (json_output == NULL) return GS_ERROR_NOT_FOUND;
    if (!cJSON_IsNumber(json_output)) return GS_ERROR_TYPE;
    if (json_output->valuedouble > INT32_MAX) return GS_ERROR_OVERFLOW;
    if (json_output->valuedouble < INT32_MIN) return GS_ERROR_OVERFLOW;

    *output = json_output->valueint;

    return GS_OK;
}

gs_error_t gs_json_parse_uint32(const cJSON* const json, const char* const label, uint32_t* output)
{
    const cJSON* json_output = cJSON_GetObjectItemCaseSensitive(json, label);
    
    if (json_output == NULL) return GS_ERROR_NOT_FOUND;
    if (!cJSON_IsNumber(json_output)) return GS_ERROR_TYPE;
    if (json_output->valuedouble > UINT32_MAX) return GS_ERROR_OVERFLOW;
    if (json_output->valuedouble < 0) return GS_ERROR_OVERFLOW;

    *output = json_output->valuedouble;

    return GS_OK;
}

gs_error_t gs_json_parse_float(const cJSON* const json, const char* const label, float* output)
{
    const cJSON* json_output = cJSON_GetObjectItemCaseSensitive(json, label);
    
    if (json_output == NULL) return GS_ERROR_NOT_FOUND;
    if (!cJSON_IsNumber(json_output)) return GS_ERROR_TYPE;
    if (json_output->valuedouble > FLT_SIMPLE_MAX) return GS_ERROR_OVERFLOW;
    if (json_output->valuedouble < FLT_SIMPLE_MIN) return GS_ERROR_OVERFLOW;

    *output = json_output->valuedouble;

    return GS_OK;
}

gs_error_t gs_json_parse_double(const cJSON* const json, const char* const label, double* output)
{
    const cJSON* json_output = cJSON_GetObjectItemCaseSensitive(json, label);
    
    if (json_output == NULL) return GS_ERROR_NOT_FOUND;
    if (!cJSON_IsNumber(json_output)) return GS_ERROR_TYPE;
    if (json_output->valuedouble > DBL_SIMPLE_MAX - 1.0) return GS_ERROR_OVERFLOW;
    if (json_output->valuedouble < DBL_SIMPLE_MIN + 1.0) return GS_ERROR_OVERFLOW;

    *output = json_output->valuedouble;

    return GS_OK;
}

gs_error_t gs_json_parse_string(const cJSON* const json, const char* const label, char** output)
{
    const cJSON* json_output = cJSON_GetObjectItemCaseSensitive(json, label);
    
    if (json_output == NULL) return GS_ERROR_NOT_FOUND;
    if (!cJSON_IsString(json_output) || json_output->valuestring == NULL) return GS_ERROR_TYPE;

    *output = json_output->valuestring;

    return GS_OK;
}

gs_error_t gs_json_parse_bool(const cJSON* const json, const char* const label, uint8_t* output)
{
    const cJSON* json_output = cJSON_GetObjectItemCaseSensitive(json, label);
    
    if (json_output == NULL) return GS_ERROR_NOT_FOUND;
    if (!cJSON_IsBool(json_output)) return GS_ERROR_TYPE;
    
    *output = json_output->valueint;

    return GS_OK;
}

gs_error_t gs_json_parse_array(const cJSON* const json, const char* const label, cJSON** output, int* output_size)
{
    *output = cJSON_GetObjectItemCaseSensitive(json, label);
    
    if (*output == NULL) return GS_ERROR_NOT_FOUND;
    if (!cJSON_IsArray(*output)) return GS_ERROR_TYPE;

    *output_size = cJSON_GetArraySize(*output);

    return GS_OK;
}
