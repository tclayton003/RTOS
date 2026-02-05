/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
 * Interface for initializing and reading from ADC.
 * Currently locked to ADC0.
 */

/* --- includes ----------------------------------------------------------*/

#include <gs/embed/asf/samc/drivers/adc.h> // first - ensure self contained
#include <gs/util/log.h>
#include <gs/util/sem.h>
#include <gs/util/string.h>
#include <gs/embed/asf/samc/convert_asf_status_code.h>

/* --- definitions -------------------------------------------------------*/

#define MAX_ADC 1

// ADC module instance
static struct adc_module adc_module[MAX_ADC];

// Lock for ADC
static gs_sem_t adc_sem;

/* --- code --------------------------------------------------------------*/

// Read value from ADC - LOCK must be obtained before
static gs_error_t gs_adc_read_no_lock(struct adc_module * module, uint16_t * value)
{
    adc_start_conversion(module);

    enum status_code status;
    do {
        /* Wait for conversion to be done and read out result */
        status = adc_read(module, value);
    } while (status == STATUS_BUSY);

    if (status != STATUS_OK) {
        log_warning("%s: adc_read() failed, status=%d", __FUNCTION__, status);
    }

    return gs_convert_asf_status_code(status);
}

gs_error_t gs_adc_init(void)
{
    gs_error_t error = GS_OK;
    if (adc_sem == NULL) {
        error = gs_sem_create(1, &adc_sem);

        memset(adc_module, 0, sizeof(adc_module));
    }
    return error;
}

static inline Adc * adc_hw(gs_adc_t adc)
{
    switch (adc) {
        case GS_ADC0: return ADC0;
        case GS_ADC1: return ADC1;
    }
    return NULL;
}

/**
   Read from ADC.
   - lock
   - configure
   - read 1. and ignore value
   - read 2. and return value
*/
gs_error_t gs_adc_read(gs_adc_t adc, struct adc_config * config, uint16_t * value)
{
    if (adc >= MAX_ADC) {
        return GS_ERROR_HANDLE;
    }

    gs_error_t error = gs_sem_wait(adc_sem, 1000);
    if (error == GS_OK) {

        struct adc_module * module = &adc_module[adc];

        // Configure and enable ADC
        {
            enum status_code status = adc_init(module, adc_hw(adc), config);
            if (status != STATUS_OK) {
                log_warning("%s: adc_init() failed, status=%d", __FUNCTION__, status);
            }
            error = gs_convert_asf_status_code(status);
        }
        if (error == GS_OK) {

            adc_enable(module);

            // ignore first read
            (void) gs_adc_read_no_lock(module, value);

            // use second read
            error = gs_adc_read_no_lock(module, value);

            adc_disable(module);
        }
        gs_sem_post(adc_sem);
    }

    return error;
}
