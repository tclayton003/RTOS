#ifndef PARAM_PARAM_INDEX_H
#define PARAM_PARAM_INDEX_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <param/param_types.h>
#include <gs/param/host_table.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint8_t param_index_size(void)
{
    return gs_param_table_max_instances();
}

static inline param_index_t * param_ptr(param_mem mem)
{
    return (param_index_t *) gs_param_table_by_id(mem);
}

static inline param_index_t * param_ptr_by_table_name(const char * name)
{
    return (param_index_t *) gs_param_table_by_name(name);
}

static inline gs_error_t param_index_set(param_mem mem_id, param_index_t table)
{
    const gs_param_table_create_t create = {.name = table.name,
                                            .id = mem_id,
                                            .rows = (const gs_param_table_row_t *) table.table,
                                            .row_count = table.count,
                                            .memory = table.physaddr,
                                            .memory_size = table.size,
                                            //gs_param_table_initializer_function_t initializer;
                                            .callback = (gs_param_callback_func_t) table.callback,
                                            //gs_param_function_interface_t * function;
                                            //const char * stores;
                                            //const char * auto_persist;
                                            //uint32_t flags;
    };
    return gs_param_table_create(&create, NULL);
}

static inline const char * param_table_name(const param_index_t * mem)
{
    return gs_param_table_name((gs_param_table_instance_t *) mem);
}

#ifdef __cplusplus
}
#endif
#endif
