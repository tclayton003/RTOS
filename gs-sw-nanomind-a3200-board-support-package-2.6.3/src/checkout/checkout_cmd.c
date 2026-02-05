/* Copyright (c) 2019-2019 GomSpace A/S. All rights reserved. */

#include "checkout_cmd.h"
#include <gs/a3200/hmc5843.h>
#include <gs/a3200/mpu3300.h>
#include <gs/a3200/lm71.h>
#include <gs/a3200/pwr_switch.h>
#include <gs/thirdparty/fram/fm33256b.h>
#include <gs/thirdparty/flash/spn_fl512s.h>
#include <gs/util/drivers/i2c/master.h>
#include <gs/util/bytebuffer.h>
#include <gs/util/vmem_test.h>
#include <gs/util/thread.h>
#include <gs/util/time.h>
#include <gs/embed/drivers/sys/memory.h>
#include <gs/csp/csp.h>
#include <gs/embed/asf/drivers/i2c/i2c.h>
#include <stdlib.h>

#define LM71_SPI_TIMEOUT_MS 1000

gs_error_t gs_checkout_cmd_test_vmem_partition(gs_command_context_t* ctx, const char *vmem_par_name){
    gs_address_t start_addr;
    size_t mem_size;

    const gs_vmem_t *vmem_par = gs_vmem_get_by_name(vmem_par_name);

   /* If partition is not in vmem, check if sdram */
    if (vmem_par == NULL) {
        if(!strcmp("sdram",vmem_par_name)){
            start_addr.p = gs_mem_get_ext_ram_size(&mem_size);
        }
        else{
            fprintf(ctx->out, "ERROR: VMEM partition <%s> not found\r\n", ctx->argv[1]);
            return GS_ERROR_NOT_FOUND;
        }
    } else {
        /*Found vmem partition, use vmem adress/size */
        start_addr =  vmem_par->virtmem;
        mem_size = vmem_par->size;
    }

    /* 1. Unlock the VMEM partition before start */
    gs_vmem_lock_by_name(ctx->argv[1], false);

    /* 2. Do data bus test */
    gs_error_t test_res = gs_vmem_test_data_bus_test(start_addr, ctx->out);

    if (GS_OK != test_res) {
        fprintf(ctx->out, "data_bus_test() %i\n\r", test_res);
        return test_res;
    }
    gs_command_set_output_printf(ctx, "", "Data bus test", "OK");

    /* 3. Do address bus test */
    test_res = gs_vmem_test_addr_bus_test(start_addr, mem_size, ctx->out);
    if (GS_OK != test_res) {
        fprintf(ctx->out, "addr_bus_test() %i\n\r", test_res);
        return test_res;
    }
    gs_command_set_output_printf(ctx, "", "Address bus test", "OK");

    /* 4. Do single cell test */
    test_res = gs_vmem_test_cell_test(start_addr, mem_size, ctx->out);
    if (GS_OK != test_res) {
        fprintf(ctx->out, "cell_test() %i\n\r", test_res);
        return test_res;
    }
    gs_command_set_output_printf(ctx, "", "Cell test", "OK");

    /* 5. Do speed test */
    test_res = gs_vmem_test_speed_test(start_addr, mem_size, ctx->out);
    if (GS_OK != test_res) {
        fprintf(ctx->out, "speed_test() %i\n\r", test_res);
        return test_res;
    }
    gs_command_set_output_printf(ctx, "", "Speed test", "OK");

    /* 7. Do lock test */
    const gs_vmem_t * mem = gs_vmem_get_by_name(ctx->argv[1]);
    if (mem) {
        if (mem->drv && mem->drv->lock) {
            test_res = gs_vmem_test_lock_test(start_addr, mem_size, ctx->argv[1], ctx->out);
            if (GS_OK != test_res) {
                fprintf(ctx->out, "lock_test() %i\n\r", test_res);
                return test_res;
            }
        gs_command_set_output_printf(ctx, "", "Lock test", "OK");
        }
    }

    return GS_OK;
}

gs_error_t gs_checkout_cmd_memory_all_handler(gs_command_context_t* ctx){
    gs_error_t error;

    fprintf(ctx->out, "\r\nfram ->\r\n");
    error = gs_checkout_cmd_test_vmem_partition(ctx, "fram");

    if (GS_OK==error){
        fprintf(ctx->out, "\r\nsdram ->\r\n");
        error = gs_checkout_cmd_test_vmem_partition(ctx, "sdram");
    }

    return error;
}

gs_error_t gs_checkout_cmd_memory_handler(gs_command_context_t* ctx){
    return gs_checkout_cmd_test_vmem_partition(ctx, ctx->argv[1]);
}

gs_error_t gs_checkout_cmd_gyro_handler(gs_command_context_t * context){
    gs_error_t error = gs_mpu3300_selftest();

    if (GS_OK == error){
        gs_command_set_output_printf(context, "", "Gyro test", "OK");
    }

    return error;
}

gs_error_t gs_checkout_cmd_hmc5843_handler(gs_command_context_t * context){
    gs_hmc5843_data_t data;
    gs_error_t error =  gs_hmc5843_selftest(&data);

    if (GS_OK == error){
        gs_command_set_output_printf(context, "", "hmc5843 test", "OK");
    }

    return error;
}

gs_error_t gs_checkout_cmd_temperature_handler(gs_command_context_t * context){
    // Read board temperature sensors
    int16_t sensor1, sensor2;
    float gyro_temp;
    gs_error_t error;
    error=gs_lm71_read_temp(GS_A3200_SPI_SLAVE_LM71_0, LM71_SPI_TIMEOUT_MS, &sensor1);
    if (GS_OK != error){
        return error;
    }

    error=gs_lm71_read_temp(GS_A3200_SPI_SLAVE_LM71_1, LM71_SPI_TIMEOUT_MS, &sensor2);
    if (GS_OK != error){
        return error;
    }

    // Read gyroscope temperature
    error=gs_mpu3300_read_temp(&gyro_temp);
    if (GS_OK !=error){
        return error;
    }

    fprintf(context->out, "Temp1: %.1f, Temp2 %.1f, Gyro temp: %.2f\r\n",
            sensor1/10., sensor2/10., gyro_temp);

    return GS_OK;
}

gs_error_t gs_checkout_cmd_flash_handler(gs_command_context_t * context){
    const int test_runs = 2;
    uint32_t addr = 0x0;
    uint32_t addrTop = SPN_FL512S_SIZE-SPN_FL512S_SECTOR_SIZE;
    int32_t verifyResult;
    uint32_t write_times[SPN_FL512S_DIES * test_runs];
    uint32_t read_times[SPN_FL512S_DIES * test_runs];
    uint32_t write_times_index = 0;
    uint32_t read_times_index = 0;

    gs_error_t error = GS_OK;
    uint8_t  *testData, * testDataInv, *testDataRead;

    /* Allocate memory for test patterns */
    testData = malloc(SPN_FL512S_PAGE_SIZE);
    if (!testData){
        fprintf(context->out, "Unable to allocate %d bytes\r\n", SPN_FL512S_PAGE_SIZE);
        error = GS_ERROR_ALLOC;
    }

    if (GS_OK == error){
        testDataInv = malloc(SPN_FL512S_PAGE_SIZE);
        if (!testDataInv){
            fprintf(context->out, "Unable to allocate %d bytes\r\n", SPN_FL512S_PAGE_SIZE);
            error = GS_ERROR_ALLOC;
        }
    }

    if (GS_OK == error){
        testDataRead = malloc(SPN_FL512S_PAGE_SIZE);
        if (!testDataRead){
            fprintf(context->out, "Unable to allocate %d bytes\r\n", SPN_FL512S_PAGE_SIZE);
            error = GS_ERROR_ALLOC;
        }
    }

    if (GS_OK == error){
        /* Generate test patterns */
        for (int i = 0 ; i<SPN_FL512S_PAGE_SIZE ; i++)
        {
            *(testData+i) = 0x5A;
            *(testDataInv+i) = 0xA5;
        }
    }

    /* Run test twice. flipping data pattern in second run*/
    for (uint8_t k = 0; k< test_runs; k++){
        if (GS_OK != error){
            break;
        }

        /* Test all dies */
        for (uint8_t partition = 0; partition < SPN_FL512S_DIES ; partition ++){

            /* Erase blocks used for testing */
            error = spn_fl512s_erase_block(partition, addr);
            if (GS_OK != error){
                fprintf(context->out, "Erase block failed, partition %d, addr: 0x%08"PRIx32"\r\n", partition, addr);
                break;
            }

            error = spn_fl512s_erase_block(partition, addrTop);
            if (GS_OK != error){
                fprintf(context->out, "Erase block failed, partition %d, addr: 0x%08"PRIx32"\r\n", partition, addrTop);
                break;
            }

            /* Write data to flash*/
            uint32_t time_before = gs_time_rel_ms();
            for (uint16_t i=0 ; i< (SPN_FL512S_SECTOR_SIZE/SPN_FL512S_PAGE_SIZE) ; i++){
                error = spn_fl512s_write_data(partition, addr+(i*SPN_FL512S_PAGE_SIZE), testData, SPN_FL512S_PAGE_SIZE);
                if (GS_OK != error){
                    fprintf(context->out, "Write failed, partition %d, addr: 0x%08"PRIx32"\r\n", partition, addr+(i*SPN_FL512S_PAGE_SIZE));
                    break;
                }
            }

            for (uint16_t i=0 ; i< (SPN_FL512S_SECTOR_SIZE/SPN_FL512S_PAGE_SIZE) ; i++){
                error = spn_fl512s_write_data(partition,  addrTop+(i*SPN_FL512S_PAGE_SIZE), testDataInv, SPN_FL512S_PAGE_SIZE);
                if (GS_OK != error){
                    fprintf(context->out, "Write failed, partition %d, addr: 0x%08"PRIx32"\r\n", partition, addrTop+(i*SPN_FL512S_PAGE_SIZE));
                    break;
                }
            }
            uint32_t time_now = gs_time_rel_ms();
            write_times[write_times_index++] = gs_time_diff_ms(time_before, time_now);

            /* Verify data in flash*/
            time_before = gs_time_rel_ms();
            for (uint16_t i=0 ; i<(SPN_FL512S_SECTOR_SIZE/SPN_FL512S_PAGE_SIZE) ; i++){
                error = spn_fl512s_read_data(partition, addr+(i*SPN_FL512S_PAGE_SIZE), testDataRead, SPN_FL512S_PAGE_SIZE);
                if (GS_OK != error){
                    fprintf(context->out, "Read failed, partition %d, addr: 0x%08"PRIx32"\r\n", partition, addr+(i*SPN_FL512S_PAGE_SIZE));
                    break;
                }

                verifyResult = memcmp(testDataRead,testData,SPN_FL512S_PAGE_SIZE);
                if (verifyResult){
                    error = GS_ERROR_DATA;
                    fprintf(context->out, "Data verification failed, part. %d, addr: 0x%08"PRIx32"\r\n", partition, addr+(i*SPN_FL512S_PAGE_SIZE));
                    break;
                }
            }

            for (uint16_t i=0 ; i<(SPN_FL512S_SECTOR_SIZE/SPN_FL512S_PAGE_SIZE) ; i++){
                error = spn_fl512s_read_data(partition, addrTop+(i*SPN_FL512S_PAGE_SIZE), testDataRead, SPN_FL512S_PAGE_SIZE);
                if (GS_OK != error){
                    fprintf(context->out, "Read failed, partition %d, addr: 0x%08"PRIx32"\r\n", partition, addrTop+(i*SPN_FL512S_PAGE_SIZE));
                    break;
                }

                verifyResult = memcmp(testDataRead,testDataInv,SPN_FL512S_PAGE_SIZE);
                if (verifyResult){
                    error = GS_ERROR_DATA;
                    fprintf(context->out, "Data verification failed, part. %d, addr: 0x%08"PRIx32"\r\n", partition, addrTop+(i*SPN_FL512S_PAGE_SIZE));
                    break;
                }
            }
            time_now = gs_time_rel_ms();
            read_times[read_times_index++] = gs_time_diff_ms(time_before, time_now);
        }

        /* Flip addresses for second pass. This will inverse data pattern */
        uint32_t tmp_addr;

        tmp_addr = addrTop;
        addrTop = addr;
        addr = tmp_addr;
    }

    free(testData);
    free(testDataInv);
    free(testDataRead);

    if (GS_OK == error){
        uint32_t write_speed = (SPN_FL512S_SECTOR_SIZE * 2 * 4) /
                                (write_times[0] + write_times[1] + write_times[2] + write_times[3]); // Bytes/ms = kB/s
        uint32_t read_speed = (SPN_FL512S_SECTOR_SIZE * 2 * 4) /
                              (read_times[0] + read_times[1] + read_times[2] + read_times[3]); // Bytes/ms = kB/s
        const unsigned int write_limit = 550;
        if (write_speed < write_limit) {
            fprintf(context->out, "ERROR: Flash write bw < %u kB/s - bw measured: %"PRIu32"\n\r",
                    write_limit, write_speed);
            error = GS_ERROR_IO;
        }
        const unsigned int read_limit = 300;
        if (read_speed < read_limit) {
            fprintf(context->out, "ERROR: Flash read bw < %u kB/s - bw measured: %"PRIu32"\n\r",
                    read_limit, read_speed);
            error = GS_ERROR_IO;
        }
        if (error == GS_OK) {
            gs_command_set_output_printf(context, "", "Flash test", "OK");
            gs_command_set_output_printf(context, "", "Write bw [kB/s]", "%"PRIu32"", write_speed);
            gs_command_set_output_printf(context, "", "Read bw [kB/s]", "%"PRIu32"", read_speed);
        }
    }

    /* Test PWR switch to flash */
    gs_a3200_pwr_switch_set(GS_A3200_PWR_SD, 0);
    gs_thread_sleep_ms(100); // Allow time to turn off
    error = spn_fl512s_erase_block(0, 0x00);
    if (error == GS_OK) {
        fprintf(context->out, "ERROR: Flash PWR switch did not turn off - spn_fl512s_erase_block(%i)\n\r", error);
        error = GS_ERROR_IO;
    } else {
        gs_a3200_pwr_switch_set(GS_A3200_PWR_SD, 1);
        gs_thread_sleep_ms(100); // Allow time to turn on
        error = spn_fl512s_erase_block(0, 0x00);
        if (error != GS_OK) {
            fprintf(context->out, "ERROR: Flash PWR switch did not turn on - spn_fl512s_erase_block(%i)\n\r", error);
        } else {
            gs_command_set_output_printf(context, "", "Flash PWR switch test", "OK");
        }
    }
    return error;
}

gs_error_t gs_checkout_cmd_auto_handler(gs_command_context_t* ctx){
    gs_error_t error = GS_OK;

    fprintf(ctx->out, "\r\n***** Identification\r\n");
    fprintf(ctx->out, "Revision: %s\r\n",csp_get_revision());

    if (GS_OK == error){
        fprintf(ctx->out, "\r\n***** Testing memory\r\n");
        error = gs_checkout_cmd_memory_all_handler(ctx);
    }

    if (GS_OK == error){
        fprintf(ctx->out, "\r\n***** Testing gyro\r\n");
        error = gs_checkout_cmd_gyro_handler(ctx);
    }

    if (GS_OK == error){
        fprintf(ctx->out, "\r\n***** Testing hmc5843\r\n");
        error = gs_checkout_cmd_hmc5843_handler(ctx);
    }

    if (GS_OK == error){
        fprintf(ctx->out, "\r\n***** Testing temperature sensors\r\n");
        error = gs_checkout_cmd_temperature_handler(ctx);
    }

    if (GS_OK == error){
        fprintf(ctx->out, "\r\n***** Testing spansion flash\r\n");
        error = gs_checkout_cmd_flash_handler(ctx);
    }

    return error;
}

gs_error_t gs_checkout_gssb_i2c(gs_command_context_t* ctx)
{
    const uint8_t txbuf[] = "A3200-BSP";
    const uint8_t rx_expect[] = "AARDVARK";
    uint8_t rxbuf[GS_ARRAY_SIZE(rx_expect)];
    gs_error_t error = gs_i2c_master_transaction(2, 5, txbuf, GS_ARRAY_SIZE(txbuf), rxbuf, GS_ARRAY_SIZE(rx_expect) - 1, 1000);
    if (error != GS_OK) {
        fprintf(ctx->out, "Error: gs_i2c_master_transaction(%i)\n\r", error);
        return error;
    }
    return GS_OK;
}

static const gs_command_t GS_COMMAND_SUB checkout_sub_commands[] = {
    {
        .name="all",
        .help="Execute all checkout tests",
        .handler=gs_checkout_cmd_auto_handler,
    },
    {
        .name="temperature",
        .help="Read temeratures from sensors",
        .handler=gs_checkout_cmd_temperature_handler,
    },
    {
        .name="memory",
        .help="Run test on ext memory/flash",
        .usage="<vmem partition>",
        .mandatory_args=1,
        .handler=gs_checkout_cmd_memory_handler,
    },
    {
        .name="flash",
        .help="Run test on external flash",
        .handler=gs_checkout_cmd_flash_handler,
    },
    {
        .name="gyro",
        .help="Run test on gyro",
        .handler=gs_checkout_cmd_gyro_handler,
    },
    {
        .name="hmc5843",
        .help="Run test on magnetometer",
        .handler=gs_checkout_cmd_hmc5843_handler,
    },
    {
        .name="gssb_i2c",
        .help="Send to and read from connected slave",
        .handler=gs_checkout_gssb_i2c,
    },
};

static const gs_command_t GS_COMMAND_ROOT checkout_root_commands[] = {
    {
        .name="checkout",
        .help="Checkout tests",
        .chain=GS_COMMAND_INIT_CHAIN(checkout_sub_commands),
    },
};

gs_error_t gs_checkout_register_commands(void)
{
    return GS_COMMAND_REGISTER(checkout_root_commands);
}
