#ifndef PARAM_PARAM_HOST_H
#define PARAM_PARAM_HOST_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <param/param_index.h>
#include <param/rparam_client.h>
#include <gs/param/host_table.h>
#include <gs/util/string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PARAM_PASTE(name, native_type, param_type)                      \
    static inline native_type param_get_##name(uint16_t addr, param_mem mem_id) { \
        native_type ret = 0;                                            \
        gs_param_get_##name(gs_param_table_by_id(mem_id), addr, &ret, 0); \
        return ret;                                                     \
    }									\
    static inline native_type param_get_##name##_fromidx(uint16_t addr, param_index_t * mem) { \
        native_type ret = 0;                                            \
        gs_param_get_##name((gs_param_table_instance_t *) mem, addr, &ret, 0); \
        return ret;                                                     \
    }									\
    static inline void param_set_##name(uint16_t addr, param_mem mem_id, native_type value) { \
        gs_param_set_##name(gs_param_table_by_id(mem_id), addr, value, 0); \
    }									\
    static inline void param_set_##name##_persist(uint16_t addr, param_mem mem_id, native_type value) { \
        gs_param_set_##name(gs_param_table_by_id(mem_id), addr, value, GS_PARAM_F_AUTO_PERSIST); \
    }									\
    static inline void param_set_##name##_nocallback(uint16_t addr, param_mem mem_id, native_type value) { \
        gs_param_set_##name(gs_param_table_by_id(mem_id), addr, value, GS_PARAM_F_NO_CALLBACK); \
    }
    
PARAM_PASTE(bool, bool, GS_PARAM_BOOL);
PARAM_PASTE(uint8, uint8_t, GS_PARAM_UINT8);
PARAM_PASTE(uint16, uint16_t, GS_PARAM_UINT16);
PARAM_PASTE(uint32, uint32_t, GS_PARAM_UINT32);
PARAM_PASTE(uint64, uint64_t, GS_PARAM_UINT64);
PARAM_PASTE(int8, int8_t, GS_PARAM_INT8);
PARAM_PASTE(int16, int16_t, GS_PARAM_INT16);
PARAM_PASTE(int32, int32_t, GS_PARAM_INT32);
PARAM_PASTE(int64, int64_t, GS_PARAM_INT64);
PARAM_PASTE(double, double, GS_PARAM_DOUBLE);
PARAM_PASTE(float, float, GS_PARAM_FLOAT);

static inline gs_error_t param_set_string(uint16_t addr, param_mem mem_id, const char * value)
{
    return gs_param_set_string(gs_param_table_by_id(mem_id), addr, value, 0);
}

static inline gs_error_t param_set_string_nocallback(uint16_t addr, param_mem mem_id, const char * value)
{
    return gs_param_set_string(gs_param_table_by_id(mem_id), addr, value, GS_PARAM_F_NO_CALLBACK);
}

static inline gs_error_t param_set_string2(uint16_t addr, param_mem mem_id, const char * value, uint8_t flags)
{
    return gs_param_set_string(gs_param_table_by_id(mem_id), addr, value, flags);
}
    
static inline void * param_read_addr(uint16_t addr, param_index_t * mem, size_t size)
{
    size_t memory_size;
    uint8_t * pointer = gs_param_table_get_memory((gs_param_table_instance_t *) mem, &memory_size);
    if (pointer && ((addr + size) < memory_size)) {
        pointer += addr;
    }
    return pointer;
}

static inline const char * param_get_string(uint16_t addr, param_mem mem_id)
{
    return param_read_addr(addr, param_ptr(mem_id), 0);
}

#ifdef __cplusplus
}
#endif
#endif
