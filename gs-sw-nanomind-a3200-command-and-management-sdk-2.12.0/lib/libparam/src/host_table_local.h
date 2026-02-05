#ifndef SRC_HOST_TABLE_LOCAL_H
#define SRC_HOST_TABLE_LOCAL_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/param/host_table.h>
#include <gs/param/internal/types.h>

#define GS_PARAM_MAX_TABLE_NAME 25

gs_param_table_instance_t * gs_param_table_raw(gs_param_table_id_t table_id);

gs_error_t gs_param_table_check_all(gs_param_table_instance_t * tinst, uint32_t flags);

/**
   Save table to all configured store(s).

   @param[in] tinst table instance.
   @param[in] flags flags.
   @return_gs_error_t
*/
gs_error_t gs_param_table_save_all(gs_param_table_instance_t * tinst,
                                   uint32_t flags);

/**
   Iterate all tables, where the table name matches the \a name_pattern.
*/
gs_error_t gs_param_table_iterate(const char * name_pattern, void * ctx, bool(*iter)(void *ctx, gs_param_table_instance_t * tinst));

gs_error_t gs_param_host_init(void);

#endif
