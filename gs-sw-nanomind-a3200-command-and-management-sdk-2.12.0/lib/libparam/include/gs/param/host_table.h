#ifndef GS_PARAM_HOST_TABLE_H
#define GS_PARAM_HOST_TABLE_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   Host table API.
*/

#include <gs/param/table.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Return max (configured) table instances.
*/
unsigned int gs_param_table_max_instances(void);

/**
   Returns table instance by table id.
   @param[in] table_id table id.
   @return table instance or NULL if not found.
*/
gs_param_table_instance_t * gs_param_table_by_id(gs_param_table_id_t table_id);

/**
   Returns table instance by table name.
   @param[in] name table name.
   @return table instance or NULL if not found.
*/
gs_param_table_instance_t * gs_param_table_by_name(const char * name);

/**
   Returns table instance by table name.
   @param[in] name table name.
   @param[out] return_tinst table instance.
   @return_gs_error_t
*/
gs_error_t gs_param_table_by_name2(const char * name, gs_param_table_instance_t ** return_tinst);

/**
   Table initializer function.
   @see gs_param_table_create()
*/
typedef gs_error_t (*gs_param_table_initializer_function_t)(gs_param_table_instance_t * tinst);

/**
   Create parameters for gs_param_table_create().
*/
typedef struct {
    /**
       Name.
       @note If specified, the pointer/string must remain valid as long as the table exists.
    */
    const char * name;
    /**
       Id.
    */
    gs_param_table_id_t id;
    /**
       Rows (specifications).
       @note If specified, the pointer must remain valid as long as the table exists.
    */
    const gs_param_table_row_t * rows;
    /**
       Number of rows.
    */
    size_t row_count;
    /**
       Memory.
       @note If specified, the pointer must remain valid as long as the table exists.
    */
    void * memory;
    /**
       Memory size.
       The actual memory footprint of the table may be smaller.
    */
    size_t memory_size;
    /**
       Default values for initializing table parameters.
       @note The size must be at least that of the table definition, eg \a rows.
    */
    const void * default_values;
    /**
       Function for initializing table parameters.
       @note If using gs_param_set(), use the flag #GS_PARAM_F_NO_CALLBACK to avoid callbacks.
       Unless the table memory is allocated (by #GS_PARAM_TABLE_F_ALLOC_MEMORY) or initialized using \a default_values, the table memory hasn't
       been touched and the initializer must set all values.
    */
    gs_param_table_initializer_function_t initializer;
    /**
       Parameter update callback.
       Called when a parameter is set (even if the value is the same).
       The \a callback will also be called after load of the table, unless suppresssed by #GS_PARAM_F_NO_CALLBACK.
    */
    gs_param_callback_func_t callback;
    /**
       Function interface.
    */
    gs_param_function_interface_t * function;
    /**
       CVS list of stores - where the table is to be saved and loaded from.
       @note If specified, the pointer/string must remain valid as long as the table exists.
    */
    const char * stores;
    /**
       Auto-persit store - only one store supported.
       @note If specified, the pointer/string must remain valid as long as the table exists.
    */
    const char * auto_persist;
    /**
       Flags.
       #GS_PARAM_TABLE_F_ALLOC_MEMORY - allocate memory.
    */
    uint32_t flags;
} gs_param_table_create_t;

/**
   Create and initialize global table.

   The parameter system supports a number of global tables. Once a global table is created, it can be accessed using the
   table id (gs_param_table_by_id()) or name (gs_param_table_by_name()).

   A global table must have a \a name.

   Table \a name and \a id must be unique among all created tables.

   Table memory is initialized using gs_param_table_memory_initialize().

   The parameter update callback will not be called, unless done so from specified initializer function (which is not recommended).
   Callbacks will be called by parameter load or set.

   @param[in] create create options.
   @param[out] return_tinst created table instance.
   @return_gs_error_t
*/
gs_error_t gs_param_table_create(const gs_param_table_create_t * create,
                                 gs_param_table_instance_t ** return_tinst);

/**
   Create and initialize anonymous table.

   Compared to gs_param_table_create(), this table can only be accessed using the provided table instance.

   Table memory is initialized using gs_param_table_memory_initialize();

   The parameter update callback will not be called, unless done so from specified initializer function (which is not recommended).
   Callbacks will be called by parameter load or set.

   Use gs_param_table_free() to free table resources.

   @version 4.4
   @param[in] tinst pre-allocated table instance.
   @param[in] create create options.
   @return_gs_error_t
*/
gs_error_t gs_param_table_init(gs_param_table_instance_t * tinst,
                               const gs_param_table_create_t * create);

/**
   Return table name.

   @param[in] tinst table instance
   @return name of table or an empty string.
*/
const char * gs_param_table_get_name(const gs_param_table_instance_t * tinst);

/**
   Return table name.

   @param[in] tinst table instance
   @return name of table or an empty string.
*/
static inline const char * gs_param_table_name(const gs_param_table_instance_t * tinst)
{
    return gs_param_table_get_name(tinst);
}

/**
   Return table id.

   @param[in] tinst table instance
   @return table id
*/
gs_param_table_id_t gs_param_table_get_id(const gs_param_table_instance_t * tinst);

/**
   Load table from store.

   Auto-persits are NOT loaded.

   @param[in] tinst table instance.
   @param[in] store name of store.
   @param[in] slot name of slot in store.
   @param[in] flags flags.
   @return_gs_error_t
*/
gs_error_t gs_param_table_load(gs_param_table_instance_t * tinst,
                               const char * store, const char * slot,
                               uint32_t flags);

/**
   Load table and auto-persist data (fallback strategy).

   The table will be tried loaded from one of the configured store(s), starting with the first store in the list - this is the primary store..

   If the table cannot be loaded from any of the stores, the table will be initialized as descrbed in gs_param_table_create().

   Auto-persist data will then be loaded, unless the flag #GS_PARAM_STORE_F_SKIP_AUTO_PERSIST is specified.

   If a parameter update callback has been configured (see gs_param_table_create()), it will be called once for all addresses 
   in the table - unless the flag #GS_PARAM_F_NO_CALLBACK is specified.

   If the table cannot be loaded from it's primary store (first store in the list), an \a error will be logged.

   @param[in] tinst table instance.
   @param[in] flags flags. Use #GS_PARAM_F_NO_CALLBACK, #GS_PARAM_STORE_F_SKIP_AUTO_PERSIST.
   @return_gs_error_t
 */
gs_error_t gs_param_table_load_fallback(gs_param_table_instance_t * tinst,
                                        uint32_t flags);

/**
   Save table to store.
   @param[in] tinst table instance.
   @param[in] store name of store.
   @param[in] slot name of slot in store.
   @param[in] flags flags.
   @return_gs_error_t
*/
gs_error_t gs_param_table_save(gs_param_table_instance_t * tinst,
                               const char * store, const char * slot,
                               uint32_t flags);

/**
   Call \a callback for all addresses in table.

   @param[in] tinst table instance.
   @return_gs_error_t
*/
gs_error_t gs_param_table_call_callback_for_all_addresses(gs_param_table_instance_t * tinst);

/**
   Initialize table memory to zero.

   @note Table is not locked.

   @param[in] tinst table instance.
   @return_gs_error_t
*/
gs_error_t gs_param_table_memory_initialize_zero(gs_param_table_instance_t * tinst);

/**
   Initialize table memory (data).


   The table memory is initialized in following steps until successful:
   1. use \a default \a values specified in #gs_param_table_create_t.
   2. use \a initializer function specified in #gs_param_table_create_t.
   3. initialize all memory to zero.

   This will not trigger any parameter change callbacks, unless done by \a initializer function - not recommended.

   Table is locked udring initialization.

   @param[in] tinst table instance.
   @return_gs_error_t
*/
gs_error_t gs_param_table_memory_initialize(gs_param_table_instance_t * tinst);

/**
   Copy parameters to table from memory.

   Memory is expected to match table layout.

   @version 4.4
   @param[in] tinst table intance.
   @param[in] memory memory to copy from
   @param[in] memory_size size of \a memory.
   @param[in] flags flags to control copy.
   @param[in] allow_partial allow partial copy, i.e. \a memory_size less than table size.
   @param[out] partial_copy set if copy was partial, i.e. \a memory_size less than table size - but all copied parameters were copied consistently.
   @return #GS_OK if full table copied, or consistent partial set of parameters copied, see \a return_partial.
   @return #GS_ERROR_OVERFLOW if memory is too short/mismatch against table layout.
   @return_gs_error_t
*/
gs_error_t gs_param_table_copy_from_memory(gs_param_table_instance_t * tinst, const void * memory, size_t memory_size, uint32_t flags,
                                           bool allow_partial, bool * partial_copy);

/**
   Copy parameters to memory from table.

   Memory is expected to match table layout.

   @version 4.4
   @param[in] tinst table intance.
   @param[in] memory memory to copy from
   @param[in] memory_size size of \a memory.
   @param[in] flags flags to control copy.
   @return #GS_OK if full table copied, or consistent partial set of parameters copied.
   @return #GS_ERROR_OVERFLOW if memory is too short against table layout.
   @return_gs_error_t
*/
gs_error_t gs_param_table_copy_to_memory(gs_param_table_instance_t * tinst, void * memory, size_t memory_size, uint32_t flags);

#ifdef __cplusplus
}
#endif
#endif
