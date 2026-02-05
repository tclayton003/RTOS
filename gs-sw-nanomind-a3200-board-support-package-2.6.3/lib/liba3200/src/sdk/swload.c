/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include "../local.h"
#include <sys/stat.h>
#include "board.h"
#include <gs/embed/asf/drivers/i2c/i2c.h>
#include <gs/embed/drivers/sys/reset.h>
#include "gs/embed/asf/avr32/drivers/sys/avr32_reset_all_interrupt_settings.h"
#include <gs/util/time.h>

#define MAX_SWLOAD_COUNT          100U

static int swload_file(const char * path, void * swload_link_addr, size_t swload_max_sz)
{
    /* Open file */
    FILE *fd = fopen(path, "r");
    if (fd == NULL) {
        log_warning("File not found %s", path);
        return -1;
    }

    log_debug("File open %s", path);

    /* Read file size */
    struct stat fno;
    int result = stat(path, &fno);
    if (result != 0) {
        log_warning("stat failed on %s", path);
        fclose(fd);
        return -1;
    }
    if (fno.st_size < 0) {
        log_debug("Fstat succeeded on %s, but st_size is invalid %ld", path, (long) fno.st_size);
        fclose(fd);
        return -1;
    }

    /* Check if file size fits within swload area */
    if ((size_t)fno.st_size >= swload_max_sz) {
        log_error("File size too large %ld, max %lu", fno.st_size, (unsigned long) swload_max_sz);
        fclose(fd);
        return -1;
    }

    /* Read file */
    const uint32_t start_time = gs_time_rel_ms();
    const size_t read = fread(swload_link_addr, 1, fno.st_size, fd);
    const uint32_t ms = gs_time_diff_ms(start_time, gs_time_rel_ms());
    fclose(fd);

    if (read != (size_t)fno.st_size) {
        log_error("Failed to read file, read %u, expected %lu", (unsigned int) read, fno.st_size);
        return -1;
    }

    log_debug("Read %lu bytes in %"PRIu32" ms (%lu KBytes/sec)", (unsigned long) read, ms, (unsigned long)(read/ms));

    /* Software pre-loaded, give user 10 seconds to abort (reset and run swload) */
    log_notice("Booting image [%s] in 10 seconds...", path);
    gs_time_sleep_ms(10 * 1000);

    log_debug("Jumping to address %p", (void *) swload_link_addr);
#ifndef __linux
    gs_sys_avr32_reset_all_interrupt_settings(); //Reset/Clear interrupt settings so no pending interrupts will cause errors
#endif
    //Watchdog is left alone - it is touched every second, and timeout is 30s, so it seems unlikely that it will take 30s to reconfigure it.
    //Should be noted that it is in the server task if large changes are made.
    DISABLE_ALL_INTERRUPTS();
    void (*jump)(void) __attribute__((__noreturn__)) = (void *) swload_link_addr;
    jump();

    /* Somthing went wrong */
    return -1;
}

void * gs_a3200_sdk_swload_task(void * param)
{
    struct load_image {
        int count_addr;
        int img_addr;
    };

    struct load_image img_par[] = {{GS_A3200_CONFIGURATION_SWLOAD_CNT1, GS_A3200_CONFIGURATION_SWLOAD_IMG1},
                                   {GS_A3200_CONFIGURATION_SWLOAD_CNT2, GS_A3200_CONFIGURATION_SWLOAD_IMG2}};

    for (unsigned int idx=0; idx < GS_ARRAY_SIZE(img_par); idx++)
    {
        gs_param_table_instance_t * tinst = gs_param_table_by_id(GS_A3200_CONFIGURATION_TABLE_MEM_ID);
        unsigned int count = gs_param_get_uint16_nc(tinst, img_par[idx].count_addr, 0);
        if (count > MAX_SWLOAD_COUNT)
        {
            log_error("swload_cnt%d was set too high: %u, setting it to %u", idx, count, MAX_SWLOAD_COUNT);
            count = MAX_SWLOAD_COUNT;
        }

        if (count > 0)
        {
            char path[100];
            gs_param_get_string(tinst, img_par[idx].img_addr, path, sizeof(path), 0);

            log_notice("Trying to boot image: %s (%u times left)", path, count);
            gs_param_set_uint16(tinst, img_par[idx].count_addr, count - 1, GS_PARAM_F_AUTO_PERSIST);

            swload_file(path, (void *)SWLOAD_LINK_ADDRESS, SWLOAD_LINK_SIZE);
        }
        /* If no RAM image is started/loaded we clear reset cause - To ensure it's it not
         * read on subsequent boots */
        gs_sys_clear_reset_cause();
    }

    gs_thread_exit(NULL);
}
