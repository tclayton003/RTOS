#ifndef GS_HK_FILE_H
#define GS_HK_FILE_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

/**
   @file

   @brief Data types and functions used in housekeeping (File I/O).

   The data types defined here are internal data types to the housekeeping system.
*/


#include <gs/hk/internal/internal_config.h>

void file_create_filename(char* buf, size_t buf_size,
                          uint8_t node, uint8_t id);

/**
 * Save hk serialized and packed data to file
 * @param [in] table pointer to housekeeping table to save.
 * @param [in] filename pointer to filename
 * @param [in] max_elem maximum number of elements to save
 * @return_gs_error_t
 */
gs_error_t file_save(gs_hk_table_cfg_t* table, const char* filename,
                        uint32_t max_elem);

/**
 * Load hk serialized and packed data from file
 * @param [in] table pointer to housekeeping table to load.
 * @param [in] filename pointer to filename
 * @param [in] max_elem maximum number of elements to load
 * @return_gs_error_t
 */
gs_error_t file_load(gs_hk_table_cfg_t* table, const char* filename,
                     uint32_t max_elem);

#endif
