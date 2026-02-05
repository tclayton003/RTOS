/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

/**
   @file

   Log appender implemented using Virtual Memory (VMEM) interface.

   The data is stored in a virtual memory area and uses gs/util/vmem.h for
   read/write of data.
*/

#include <gs/util/error.h>
#include <gs/util/check.h>
#include <gs/util/string.h>
#include <gs/util/mutex.h>
#include <gs/util/vmem.h>
#include <gs/util/bytebuffer.h>
#include <gs/util/log/log.h>
#include <gs/util/log/appender/appender.h>
#include <gs/log/appender/memory.h>

#include <stdlib.h>
#include <time.h>

typedef struct memory_drv_data {
    /** Initialization status of the buffer */
    bool is_initialized;

    /** Lock for accessing memory */
    gs_mutex_t lock;

    /** Current pointer into the ring buffer. */
    uint8_t* input_ptr;

    /** Address/location of current pointer into the ringbuffer */
    uint8_t* input_ptr_addr;
    /** Begin address of the ring buffer */
    uint8_t* begin;
    /** End address of the ring buffer */
    uint8_t* end;
    /** Buffer size of the ring buffer */
    size_t bufferlen;
} memory_drv_data_t;

/**
   log memory record
*/
typedef struct __attribute__((packed)) {
    uint32_t sec;
    uint32_t nsec;
    uint8_t level;
    uint8_t len;
    char data[0];
} gs_log_memory_record_t;

static bool gs_log_append_memory_drv_is_enabled(gs_log_appender_t *appender)
{
    if (appender && appender->drv_data) {
        return ((memory_drv_data_t*)appender->drv_data)->is_initialized;
    }
    return false;
}

// NOTE: access to data[-1] and data[size] 
GS_STATIC void gs_log_drv_memory_appender_record_locked(memory_drv_data_t *d, uint8_t * data, size_t size)
{
    data[size] = size;
    --data;
    data[0] = size;
    size += 2;

    // Work on a copy
    uint8_t* input_ptr = d->input_ptr;

    // Check space in buffer
    if ((input_ptr + size) > d->end) {
        const unsigned int remain = d->end - input_ptr;
        gs_vmem_cpy(input_ptr, data, remain);
        data += remain;
        size -= remain;
        input_ptr = d->begin;
    }

    gs_vmem_cpy(input_ptr, data, size);
    input_ptr += size;
    if (input_ptr >= d->end) {
        input_ptr = d->begin;
    }

    // Update & write next ptr
    d->input_ptr = input_ptr;
    gs_vmem_cpy(d->input_ptr_addr, &d->input_ptr, sizeof(d->input_ptr));
}

static void gs_log_drv_memory_appender_clear_locked(memory_drv_data_t * drv_data)
{
    // Reset current pointer to start of buffer
    drv_data->input_ptr = drv_data->begin;
    gs_vmem_cpy(drv_data->input_ptr_addr, &drv_data->input_ptr, sizeof(drv_data->input_ptr));

    // Write 0 (zero) in previous byte to indicate no record
    const uint8_t size = 0;
    gs_vmem_cpy(drv_data->end - 1, &size, sizeof(size));
}

static gs_error_t gs_log_drv_memory_init(gs_log_appender_t *appender)
{
    const gs_log_appender_memory_config_t *config = appender->drv_config;
    GS_CHECK_ARG(config->size >= 512);

    memory_drv_data_t *drv_data = appender->drv_data;
    if (drv_data == NULL) {
        drv_data = calloc(1, sizeof(*drv_data));
        if (drv_data == NULL) {
            return GS_ERROR_ALLOC;
        }

        gs_mutex_create(&drv_data->lock);
    }

    uint8_t * start_addr = config->address;
    if (start_addr == NULL) {
        start_addr = calloc(1, config->size);
        if (start_addr == NULL) {
            gs_mutex_destroy(drv_data->lock);
            free(drv_data);
            return GS_ERROR_ALLOC;
        }
    }

    /* Store settings */
    drv_data->input_ptr_addr = start_addr;
    drv_data->begin = start_addr + sizeof(start_addr);
    drv_data->end = start_addr + config->size;
    drv_data->bufferlen = config->size - sizeof(start_addr);

    /* Read input ptr and validate it */
    gs_vmem_cpy(&drv_data->input_ptr, drv_data->input_ptr_addr, sizeof(drv_data->input_ptr));
    if ((drv_data->input_ptr < drv_data->begin) || (drv_data->input_ptr >= drv_data->end)) {
        gs_log_drv_memory_appender_clear_locked(drv_data);
    }

    drv_data->is_initialized = true;
    appender->drv_data = drv_data;

    return GS_OK;
}

static void gs_log_drv_memory_append(gs_log_appender_t *appender, gs_log_level_t level, const gs_log_group_t *group, const gs_timestamp_t * ts, const char * format, va_list va)
{
    if (!gs_log_append_memory_drv_is_enabled(appender)) {
        return;
    }

    struct {
        // cppcheck-suppress unusedStructMember
        uint32_t prefiller; // make room for the 'start' marker
        gs_log_memory_record_t rec;
        uint8_t message[100];
        // cppcheck-suppress unusedStructMember
        uint32_t postfiller; // make room for the 'end' marker
    } rec;
    
    va_list my_va;
    va_copy(my_va, va);

    rec.rec.level = level;
    rec.rec.sec = ts->tv_sec;
    rec.rec.nsec = ts->tv_nsec;

    gs_bytebuffer_t bb;
    gs_bytebuffer_init(&bb, rec.message, sizeof(rec.message));
    gs_bytebuffer_printf(&bb, "%s: ", group->name);
    gs_bytebuffer_vprintf(&bb, format, my_va);

    va_end(my_va);

    rec.rec.len = gs_bytebuffer_get_used(&bb);

    const uint8_t data_size = (sizeof(rec.rec) + rec.rec.len);

    memory_drv_data_t * drv = appender->drv_data;
    gs_mutex_lock(drv->lock);
    gs_log_drv_memory_appender_record_locked(drv, (uint8_t*) &rec.rec, data_size);
    gs_mutex_unlock(drv->lock);
}

static gs_error_t gs_log_drv_memory_info(gs_log_appender_t *appender, char *info_str, uint8_t str_size)
{
    if (info_str && str_size) {
        const gs_log_appender_memory_config_t *config = appender->drv_config;
        snprintf(info_str, str_size, "Writes to VMEM: %p (size: %"PRIu32")",
                 config->address, config->size);
    }
    return GS_OK;
}

static gs_error_t gs_log_drv_memory_clear(gs_log_appender_t *appender)
{
    if (!gs_log_append_memory_drv_is_enabled(appender)) {
        return GS_ERROR_HANDLE;
    }

    memory_drv_data_t *drv_data = appender->drv_data;

    gs_mutex_lock(drv_data->lock);
    gs_log_drv_memory_appender_clear_locked(appender->drv_data);
    gs_mutex_unlock(drv_data->lock);

    return GS_OK;
}

static gs_error_t gs_log_drv_memory_iterate_records(gs_log_appender_t *appender, void * ctx, gs_log_record_iterator_t iter)
{
    if (!gs_log_append_memory_drv_is_enabled(appender)) {
        return GS_OK;
    }

    uint8_t size8 = 0;
    size_t count = 0;
    memory_drv_data_t *d = appender->drv_data;
    uint8_t* output_ptr = d->input_ptr;

    while(1) {

        /* Decrement output_ptr by 1 */
        if (output_ptr == d->begin) {
            output_ptr = d->end - 1;
        } else {
            output_ptr--;
        }

        /* Read size of prev element */
        gs_vmem_cpy(&size8, output_ptr, 1);
        if (size8 == 0) {
            break;
        }

        /* Decrement ptr */
        if ((output_ptr - size8) < d->begin) {
            output_ptr = (output_ptr + d->bufferlen) - size8;
        } else {
            output_ptr -= size8;
        }

        /* Decrement output_ptr by 1 */
        if (output_ptr == d->begin) {
            output_ptr = d->end - 1;
        } else {
            output_ptr--;
        }

        /* Validate against buffer turnaround */
        count += size8 + 2;
        if (count >= d->bufferlen) {
            //printf("Turnaround\r\n");
            break;
        }

        /* Validate length */
        uint8_t size8_check = 0;
        gs_vmem_cpy(&size8_check, output_ptr, 1);
        if (size8_check != size8) {
            //printf("Invalid length %u %u\r\n", size8_check, size8);
            break;
        }

        output_ptr++;
        if (output_ptr == d->end) {
            output_ptr = d->begin;
        }

        /* Read out data onto stack */
        char out[size8+1];
        if (output_ptr + size8 > d->end) {
            int second = (output_ptr + size8) - d->end;
            int first = size8 - second;
            gs_vmem_cpy(out, output_ptr, first);
            gs_vmem_cpy(out + first, d->begin, second);
        } else {
            gs_vmem_cpy(out, output_ptr, size8);
        }
        out[size8] = '\0';

        gs_log_memory_record_t *record = (gs_log_memory_record_t*)out;
        gs_timestamp_t ts = {record->sec, record->nsec};

        /* Extract group name from the log message */
        char *group_end = strchr(record->data, ':');
        uint8_t msg_start = 0;
        if (group_end != NULL) {
            *group_end = '\0'; /* zero terminate group name */
            /* Message starts after group_name + zero termination + 1 space: */
            msg_start = group_end - record->data + 2;
        }

        /* Callback with log record data */
        if (iter(ctx, record->level, &ts, &record->data[0], &record->data[msg_start]) == false) {
            return GS_OK;
        }

        /* Decrement output_ptr by 1 */
        if (output_ptr == d->begin) {
            output_ptr = d->end - 1;
        } else {
            output_ptr--;
        }
    }

    return GS_OK;
}


const gs_log_appender_driver_t gs_log_appender_memory_driver = {
    .init = gs_log_drv_memory_init,
    .append = gs_log_drv_memory_append,
    .append_isr = 0,
    .info = gs_log_drv_memory_info,
    .hist = gs_log_drv_memory_iterate_records,
    .clear = gs_log_drv_memory_clear,
};
