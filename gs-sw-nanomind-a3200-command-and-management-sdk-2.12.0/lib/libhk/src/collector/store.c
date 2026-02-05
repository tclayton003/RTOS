/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/util/log.h>
#include <gs/hk/internal/internal_collector_store.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <gs/util/mutex.h>
#include <gs/util/clock.h>
#include <gs/util/check.h>
#include <gs/util/hexdump.h>
#include <gs/uthash/utlist.h>

GS_LOG_GROUP_EXTERN(log_hk);
#define LOG_DEFAULT log_hk

gs_error_t store_lock(gs_hk_store_t* store) {
    GS_CHECK_ARG(store != NULL);
    return gs_mutex_lock(store->lock);
}

gs_error_t store_unlock(gs_hk_store_t* store) {
    GS_CHECK_ARG(store != NULL);
    return gs_mutex_unlock(store->lock);
}

gs_error_t store_initialize(gs_hk_store_t* store, uint32_t sample_count,
                            size_t sample_size) {
    GS_CHECK_ARG(store != NULL);

    gs_error_t res = gs_mutex_create(&store->lock);
    store->sample_size = sample_size;
    gs_hk_sample_t* sample = NULL;
    if (res == GS_OK) {
        uint32_t i = 0;
        while (i < sample_count) {
            sample = calloc(1, sizeof(gs_hk_sample_t));
            if (sample == NULL) {
                res = GS_ERROR_ALLOC;
                break;
            }
            DL_PREPEND(store->head, sample);
            i++;
        }

        if (res == GS_OK) {
            store->tail = sample;
        }
    }

    return res;
}

gs_error_t store_add_sample(gs_hk_store_t* store, void* sample,
                            size_t sample_size, uint32_t timestamp) {

    GS_CHECK_ARG(store != NULL && sample != NULL && sample_size > 0);
    if (timestamp < store->head->timestamp) {
        log_warning("will not prepend old data, ts(head)=%"PRIu32", ts=%"PRIu32,
                    store->head->timestamp, timestamp);
        return GS_ERROR_DATA;
    }

    if (sample_size != store->sample_size) {
        log_warning("will not insert data with different size, size=%lu, "
                    "expected=%lu", (unsigned long) sample_size, (unsigned long) store->sample_size);
        return GS_ERROR_DATA;
    }

    gs_mutex_lock(store->lock);

    // truncate tail sample
    gs_hk_sample_t* new_tail = store->tail->prev;
    DL_DELETE(store->head, store->tail);

    // allocate if needed
    gs_error_t res = GS_ERROR_ALLOC;
    if (store->tail->data == NULL) {
        store->tail->data = malloc(store->sample_size);
    }

    if (store->tail->data != NULL) {
        res = GS_OK;
        // copy data into element
        memcpy(store->tail->data, sample, sample_size);
        store->tail->timestamp = timestamp;
    }

    // add new sample in front
    DL_PREPEND(store->head, store->tail);

    // and update tail pointer
    store->tail = new_tail;

    gs_mutex_unlock(store->lock);
    return res;
}

gs_error_t store_search(gs_hk_store_t* store, uint32_t timestamp,
                        gs_hk_sample_t** r_sample) {

    GS_CHECK_ARG(store != NULL);

    // find starting point
    gs_mutex_lock(store->lock);
    gs_hk_sample_t* start_sample = store->head;
    if (store->last_found != NULL &&
        store->last_found->timestamp >= timestamp) {
        start_sample = store->last_found;
    }

    gs_hk_sample_t* found = NULL;
    uint32_t delta = UINT32_MAX;

    gs_hk_sample_t* sample = NULL;
    DL_FOREACH(start_sample, sample) {
        uint32_t new_delta = abs(sample->timestamp - timestamp);

        // if smaller, we are progressing
        if (new_delta <= delta) {
            found = sample;
            delta = new_delta;
        }

        // if bigger, we have passed the best result (assuming ordered list)
        if (new_delta > delta) {
            break;
        }
    }
    gs_mutex_unlock(store->lock);

    if (found != NULL) {
        *r_sample = found;
        return GS_OK;
    }

    return GS_ERROR_NOT_FOUND;
}

gs_error_t store_free(gs_hk_store_t* store) {
    GS_CHECK_ARG(store != NULL);

    gs_hk_sample_t* s = NULL;
    gs_hk_sample_t* t = NULL;
    DL_FOREACH_SAFE(store->head, s, t) {
        DL_DELETE(store->head, s);
        free(s->data);
        free(s);
    }

    gs_mutex_destroy(store->lock);
    return GS_OK;
}

