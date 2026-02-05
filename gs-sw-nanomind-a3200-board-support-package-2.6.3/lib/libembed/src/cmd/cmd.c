/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/command.h>

#include <gs/util/gosh/command.h>
#include <gs/embed/freertos.h>
#include <gs/util/hexdump.h>
#include <gs/util/log.h>
#include <gs/util/string.h>
#include <gs/embed/drivers/sys/reset.h>
#include <gs/util/drivers/sys/memory.h>
#include <stdlib.h>
#include <errno.h>

static int cpu_reset_handler(gs_command_context_t * ctx)
{
    gs_sys_reset(GS_SYS_RESET_GOSH);
    return GS_OK;
}

static const char * task_state_to_string(eTaskState state)
{
    switch (state) {
        case eRunning:   return "RUN";
        case eReady:     return "RDY";
        case eBlocked:   return "BLK";
        case eSuspended: return "SUS";
        case eDeleted:   return "DEL";
#if (tskKERNEL_VERSION_MAJOR >= 9)
        case eInvalid:   return "INV";
#endif
    }
    return "???";
}

static int ps_handler(gs_command_context_t * ctx)
{
    uint32_t task_count = uxTaskGetNumberOfTasks();
    if (task_count == 0) {
        return GS_OK;
    }

    TaskStatus_t *status = malloc(task_count * sizeof(TaskStatus_t));
    if (status == NULL) {
        return GS_ERROR_ALLOC;
    }

    /* Generate raw status information about each task. */
    uint32_t total_runtime;
    task_count = uxTaskGetSystemState(status, task_count, &total_runtime);

    if (total_runtime == 0) {
        total_runtime = 1;
    }

    fprintf(ctx->out, "%-"GS_DEF2STRING(configMAX_TASK_NAME_LEN)"s %-6s  %s  %s  %s  %s\r\n",
            "Name",
            "State",
            "Prio(base)",
            "FreeStack(bytes)",
            "Time %",
            "Stackbase");
    for (unsigned int i = 0; i < task_count; i++) {
#if (tskKERNEL_VERSION_MAJOR >= 9) || (GS_FREERTOS_PX_STACK_BASE)
        char stack_base[30];
        snprintf(stack_base, sizeof(stack_base), "%p", status[i].pxStackBase);
#else
        const char * stack_base = "N/A";
#endif
        double percent = (status[i].ulRunTimeCounter / ((double)total_runtime / 100.0));
        fprintf(ctx->out, "%-"GS_DEF2STRING(configMAX_TASK_NAME_LEN)"s %s(%u)  %u (%u)       %6u            %7.3f %s\r\n",
                status[i].pcTaskName,
                task_state_to_string(status[i].eCurrentState), status[i].eCurrentState,
                (unsigned int) status[i].uxCurrentPriority, (unsigned int) status[i].uxBasePriority,
                (unsigned int)(status[i].usStackHighWaterMark * sizeof(StackType_t)),
                percent, stack_base);
    }

    free(status);

    return GS_OK;
}

static int peek_handler(gs_command_context_t * ctx)
{
    if (ctx->argc != 3) {
        return GS_ERROR_ARG;
    }
    uint32_t addr;
    if (gs_string_to_uint32(ctx->argv[1], &addr) != GS_OK) {
        return GS_ERROR_ARG;
    }
    uint32_t len;
    if (gs_string_to_uint32(ctx->argv[2], &len) != GS_OK) {
        return GS_ERROR_ARG;
    }

    gs_hexdump_to_stream(GS_TYPES_INT2PTR(addr), len, GS_TYPES_INT2PTR(addr), ctx->out);

    return GS_OK;
}

static int poke_handler(gs_command_context_t * ctx)
{
    if (ctx->argc != 3) {
        return GS_ERROR_ARG;
    }
    uint32_t addr;
    if (gs_string_to_uint32(ctx->argv[1], &addr) != GS_OK) {
        return GS_ERROR_ARG;
    }
    uint32_t value;
    if ((gs_string_to_uint32(ctx->argv[2], &value) != GS_OK) || (value > UINT8_MAX)) {
        return GS_ERROR_ARG;
    }

    fprintf(ctx->out, "Writing 0x%02"PRIx32"/%"PRIu32" to address 0x%"PRIx32"\r\n", value, value, addr);

    *((uint8_t*)GS_TYPES_INT2PTR(addr)) = value;

    return GS_OK;
}

static int free_handler(gs_command_context_t * ctx, bool intern)
{
    gs_mem_ram_stat_t ram_stat;
    gs_error_t error;
    if (intern) {
        error = gs_mem_get_int_ram_stat(&ram_stat);
    } else {
        error = gs_mem_get_ext_ram_stat(&ram_stat);
    }
    if (error == GS_OK) {
        gs_mem_print_ram_stat(&ram_stat, ctx->out);
    }
    return error;
}

static int free_intern(gs_command_context_t * ctx)
{
    return free_handler(ctx, true);
}

static int free_extern(gs_command_context_t * ctx)
{
    return free_handler(ctx, false);
}

static const gs_command_t GS_COMMAND_SUB cmd_free_mem_sub[] = {
    {
        .name = "int",
        .help = "free internal memory",
        .handler = free_intern,
    },{
        .name = "ext",
        .help = "free external memory",
        .handler = free_extern,
    },
};

static const gs_command_t GS_COMMAND_ROOT cmd_embed[] = {
    {
        .name = "reset",
        .help = "Reset local system",
        .handler = cpu_reset_handler,
    },{
        .name = "ps",
        .help = "Task list",
        .handler = ps_handler,
    },{
        .name = "peek",
        .help = "Read byte(s) from memory",
        .usage = "<addr> <bytes>",
        .handler = peek_handler,
    },{
        .name = "poke",
        .help = "Write byte to memory",
        .usage = "<hex addr> <value>",
        .handler = poke_handler,
    },{
        .name = "free",
        .help = "Show memory usage",
        .chain = GS_COMMAND_INIT_CHAIN(cmd_free_mem_sub),
    },
};

gs_error_t gs_embed_register_commands(void)
{
    if (GS_EMBED_ENABLE_TEST) {
        extern gs_error_t gs_embed_register_test_commands(void);
        gs_embed_register_test_commands();
    }

    return GS_COMMAND_REGISTER(cmd_embed);
}
