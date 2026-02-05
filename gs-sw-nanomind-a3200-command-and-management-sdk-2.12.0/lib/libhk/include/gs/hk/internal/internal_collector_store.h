#ifndef HK_STORE_H_
#define HK_STORE_H_
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

/**
   @file

   @brief Data types and functions used in housekeeping (Store).

   The data types defined here are internal data types to the housekeeping system.
*/

#include <param/param.h>
#include <gs/uthash/utlist.h>


typedef struct gs_hk_sample_s {
    /** Previous element. */
    struct gs_hk_sample_s* prev;
    /** Next element. */
    struct gs_hk_sample_s* next;
    /** This elements timestamp */
    uint32_t timestamp;
    /** Pointer to data sample **/
    void* data;
} gs_hk_sample_t;

typedef struct {
    gs_mutex_t lock;
    gs_hk_sample_t* head;
    gs_hk_sample_t* tail;
    gs_hk_sample_t* last_found;
    size_t sample_size;
} gs_hk_store_t;


gs_error_t store_initialize(gs_hk_store_t* store, uint32_t sample_count,
                            size_t sample_size);

gs_error_t store_add_sample(gs_hk_store_t* store, void* sample,
                            size_t sample_size, uint32_t timestamp);

gs_error_t store_search(gs_hk_store_t* store, uint32_t timestamp,
                        gs_hk_sample_t** r_sample);

gs_error_t store_free(gs_hk_store_t* store);

gs_error_t store_lock(gs_hk_store_t* store);

gs_error_t store_unlock(gs_hk_store_t* store);









//void hk_store_lock(void);
//void hk_store_unlock(void);

/**
 * Add a housekeeping table to a hk store
 * The parameters will be serialized into a new dynamic allocated memory area.
 * @param hk_table pointer to housekeeping table.
 * @return pointer to allocated pool element.
 */
//gs_hk_cache_t * hk_store_add(gs_hk_table_cfg_t* hk_table);

/**
 * Delete an element from storage pool
 * @param head pointer to storage pool (double pointer)
 * @param elem pointer to hk element to free
 */
//void hk_store_del(gs_hk_cache_t ** head, gs_hk_cache_t * elem);

/**
 * Append an element to storage pool
 * @param head pointer to storage pool (double pointer)
 * @param elem pointer to hk element to append
 */
//void hk_store_append(gs_hk_cache_t ** head, gs_hk_cache_t * elem);

/**
 * Truncate storage pool to max_elem elements
 * @param head pointer to storage pool (double pointer)
 * @param max_elem max hk elements to keep
 */
//void hk_store_truncate(gs_hk_cache_t ** head, uint32_t max_elem);

/**
 * Iterate through list of stored elements
 * @param head pointer to storage pool
 * @param elem pointer to update for each iteration
 */
//#define HK_STORE_FOREACH(head, elem) DL_FOREACH(head, elem)

/**
 * Find closest match to timestamp
 * @param head pointer to storage pool
 * @param prev pointer to previously found element (use this to save time)
 * @param timestamp time to search for
 * @return
 */
//gs_hk_cache_t * hk_store_search_timestamp(gs_hk_cache_t * head, gs_hk_cache_t * prev, uint32_t timestamp);

/**
 * Finds latest parameter measurements
 * @param head pointer to storage pool
 * @return latest parameter measurements
 */
//gs_hk_cache_t * hk_store_latest(gs_hk_cache_t * head);

#endif /* HK_STORE_H_ */
