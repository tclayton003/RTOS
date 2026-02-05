/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
/**
   @file

   Debug/test commands for use during development of newib/freertos ports.
*/

#include <errno.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <gs/util/gosh/command.h>
#include <gs/util/thread.h>
#include <gs/util/time.h>
#include <gs/util/log.h>
#include <gs/util/hexdump.h>
#include <gs/util/clock.h>
#include <gs/embed/freertos.h>
#include <gs/embed/delay.h>
#include <gs/embed/drivers/sys/clk.h>
#include <gs/embed/drivers/sys/memory.h>

#define TASK_STACK_SIZE 5000

#if (__linux__)
// fake stuff on Linux
#define IS_NEWLIB 0
#define _NEWLIB_VERSION "linux"
#define tskKERNEL_VERSION_NUMBER "linux"

typedef struct {
    int _errno;
} impure_t;
static impure_t impure;
static impure_t * _impure_ptr = &impure;
static impure_t * _global_impure_ptr = &impure;
#else
#define IS_NEWLIB 1
#endif

typedef struct {
    char name[50];
    unsigned int no;
} task_ctx_t;

static int cmd_file_test(gs_command_context_t * ctx)
{
#define DO_ASSERT(expr,ignore)   \
    if (!(expr)) {                                                      \
            fprintf(ctx->out, "%s: " GS_DEF2STRING(expr) ", file: [%s], res: %d, fd: %d, errno: %d\r\n", ignore ? "IGNORE ERROR" : "ERROR", file, res, fd, errno); \
            if (!ignore) {                                              \
                return GS_ERROR_UNKNOWN;                                \
            }                                                           \
    }                                                                   \
    res = 0; errno = 0;

#define ASSERT_IGNORE(expr)  DO_ASSERT(expr,1)
#define ASSERT(expr)         DO_ASSERT(expr,0)
    
    char filename1[100];
    snprintf(filename1, sizeof(filename1), "%s_1.txt", ctx->argv[1]);
    char filename2[100];
    snprintf(filename2, sizeof(filename2), "%s_2.txt", ctx->argv[1]);

    errno = 0;
    int res = -1;
    int fd = -1;
    const char * file;
    FILE * fp = NULL;

    file = filename1;
    ASSERT_IGNORE((res = remove(file)) == 0);
    file = filename2;
    ASSERT_IGNORE((res = unlink(file)) == 0);

    // create empty file -> stat & fstat -> remove
    {
        file = filename1;
        // cppcheck-suppress resourceLeak
        ASSERT((fp = fopen(file, "w")) != NULL);
        ASSERT(fclose(fp) == 0);

        struct stat statbuf1;
        memset(&statbuf1, 0x55, sizeof(statbuf1));
        ASSERT((res = stat(file, &statbuf1)) == 0);
        //gs_hexdump_to_stream(&statbuf1, sizeof(statbuf1), 0, ctx->out);

        ASSERT(statbuf1.st_size == 0);
        ASSERT(statbuf1.st_dev != 0);
        ASSERT(statbuf1.st_ino != 0);
        ASSERT((res = statbuf1.st_mode) & S_IFREG);
        ASSERT(statbuf1.st_atime != 0);
        ASSERT(statbuf1.st_mtime != 0);
        ASSERT(statbuf1.st_ctime != 0);
        ASSERT(statbuf1.st_blksize != 0);

#if (IS_NEWLIB)
        ASSERT(statbuf1.st_nlink == 0);
        ASSERT(statbuf1.st_uid == 0);
        ASSERT(statbuf1.st_gid == 0);
        ASSERT(statbuf1.st_rdev == 0);
        ASSERT(statbuf1.st_size == 0);
        ASSERT(statbuf1.st_spare1 == 0);
        ASSERT(statbuf1.st_spare2 == 0);
        ASSERT(statbuf1.st_spare3 == 0);
        ASSERT(statbuf1.st_blocks == 0);
        ASSERT(statbuf1.st_spare4[0] == 0);
        ASSERT(statbuf1.st_spare4[1] == 0);
#endif

        ASSERT((fd = open(file, O_RDONLY)) > 2);

        struct stat statbuf2;
        memset(&statbuf2, 0x66, sizeof(statbuf2));
        ASSERT((res = fstat(fd, &statbuf2)) == 0);
        if (memcmp(&statbuf1, &statbuf2, sizeof(statbuf1)) != 0) {
            fprintf(ctx->out, "ERROR: statbuf returned from stat and fstat are different\r\n");
            gs_hexdump_to_stream(&statbuf1, sizeof(statbuf1), 0, ctx->out);
            gs_hexdump_to_stream(&statbuf2, sizeof(statbuf2), 0, ctx->out);
            return GS_ERROR_UNKNOWN;
        }
        ASSERT(close(fd) == 0);

        ASSERT((res = remove(file)) == 0);
        ASSERT((res = remove(file)) == -1);

        ASSERT((res = stat(file, &statbuf1)) == -1);
    }

    // create file -> stat -> seek -> rename -> unlink
    {
        file = filename1;
        ASSERT((fp = fopen(file, "w")) != NULL);
        ASSERT((res = fputs("This is line ONE   \n", fp)) >= 0);
        ASSERT((res = fputs("This is line TWO   \n", fp)) >= 0);
        ASSERT((res = fputs("This is line THREE \n", fp)) >= 0);
        ASSERT(fclose(fp) == 0);

        struct stat statbuf1;
        memset(&statbuf1, 0x55, sizeof(statbuf1));
        ASSERT((res = stat(file, &statbuf1)) == 0);
        ASSERT(statbuf1.st_size == 60);

        ASSERT((fp = fopen(file, "r+")) != NULL);
        char buf[100];
        ASSERT((fgets(buf, sizeof(buf), fp)) == buf);
        ASSERT(strcmp(buf, "This is line ONE   \n") == 0);

        ASSERT(fseek(fp, 20, SEEK_CUR) == 0);
        ASSERT((fgets(buf, sizeof(buf), fp)) == buf);
        ASSERT(strcmp(buf, "This is line THREE \n") == 0);

        ASSERT(fseek(fp, 36, SEEK_SET) == 0);
        ASSERT((fputs("mod", fp)) >= 0);

        memset(buf, 0, sizeof(buf));
        ASSERT(fseek(fp, 0, SEEK_SET) == 0);
        ASSERT((res = fread(buf, 1, sizeof(buf), fp)) == 60);
        if (strcmp(buf,
                   "This is line ONE   \n"
                   "This is line TWOmod\n"
                   "This is line THREE \n") != 0) {
            fprintf(ctx->out, "ERROR - wrong file content: [%s]\r\n", buf);
            return GS_ERROR_UNKNOWN;
        }

        ASSERT(fclose(fp) == 0);

        ASSERT((res = rename(file, filename2)) == 0);
        file = filename2;

        memset(&statbuf1, 0x55, sizeof(statbuf1));
        ASSERT((res = stat(file, &statbuf1)) == 0);
        ASSERT(statbuf1.st_size == 60);

        ASSERT((res = unlink(file)) == 0);
        ASSERT((res = unlink(file)) == -1);
    }

    return GS_OK;
}

static int cmd_info(gs_command_context_t * ctx)
{
    fprintf(ctx->out, "FreeRTOS version: %s\r\n", tskKERNEL_VERSION_NUMBER);
    fprintf(ctx->out, "newlib version: %s\r\n", _NEWLIB_VERSION);
    fprintf(ctx->out, "local impure:  %p, size: %u\r\n", _impure_ptr, (unsigned int) sizeof(*_impure_ptr));
    fprintf(ctx->out, "global impure: %p, size: %u\r\n", _global_impure_ptr, (unsigned int) sizeof(*_global_impure_ptr));

    uint8_t * start = gs_mem_heap_get_start();
    uint8_t * end = gs_mem_heap_get_end();
    size_t size = (start && end) ? (end - start + 1) : 0;
    fprintf(ctx->out, "heap: %p-%p, size: %u\r\n", start, end, (unsigned int) size);

    start = gs_mem_get_int_ram_size(&size);
    end = (size) ? (start + size - 1) : NULL;
    fprintf(ctx->out, "int ram: %p-%p, size: %u\r\n", start, end, (unsigned int) size);

    start = gs_mem_get_ext_ram_size(&size);
    end = (size) ? (start + size - 1) : NULL;
    fprintf(ctx->out, "ext ram: %p-%p, size: %u\r\n", start, end, (unsigned int) size);

    fprintf(ctx->out, "System clock: %lu Hz\r\n", gs_sys_clk_get_freq());

    return GS_OK;
}

static void print_errno(FILE * out)
{
    // get all values first
    const int def_val = errno;
    const void * def_ptr = &errno;

    const int local_impure_val = _impure_ptr->_errno;
    const void * local_impure_ptr = &_impure_ptr->_errno;

    const int global_impure_val = _global_impure_ptr->_errno;
    const void * global_impure_ptr = &_global_impure_ptr->_errno;

    // the "int" errno - must be done in its own C file because of macro stuff
    int global_errno;
    void * global_errno_ptr;
    extern void gs_embed_test_get_errno(void ** , int * );
    gs_embed_test_get_errno(&global_errno_ptr, &global_errno);

    fprintf(out, "std: %d (%p), local impure: %d (%p), global impure: %d (%p), global errno: %d (%p)\r\n",
            def_val, def_ptr,
            local_impure_val, local_impure_ptr,
            global_impure_val, global_impure_ptr,
            global_errno, global_errno_ptr);
}

static int cmd_errno(gs_command_context_t * ctx)
{
    print_errno(ctx->out);
    if (ctx->argc > 1) {
        int32_t val;
        gs_string_to_int32(ctx->argv[1], &val);
        errno = val;
        print_errno(ctx->out);
    }
    return GS_OK;
}

static int cmd_malloc(gs_command_context_t * ctx)
{
    uint32_t bytes;
    if (gs_string_to_uint32(ctx->argv[1], &bytes) != GS_OK) {
        return GS_ERROR_ARG;
    }

    void * ptr = malloc(bytes);
    fprintf(ctx->out, "malloc: %p, errno: %d\r\n", ptr, errno);

    // cppcheck-suppress memleak
    return (ptr != NULL) ? GS_OK : GS_ERROR_ALLOC;
}

static int cmd_free(gs_command_context_t * ctx)
{
    void * ptr;
    if (gs_string_to_pointer(ctx->argv[1], &ptr) != GS_OK) {
        return GS_ERROR_ARG;
    }

    free(ptr);
    // cppcheck-suppress deallocuse
    fprintf(ctx->out, "free: %p, errno: %d\r\n", ptr, errno);

    return GS_OK;
}

static int spawn_tasks(gs_command_context_t * ctx, uint32_t tasks, const char * name, gs_thread_func_t func)
{
    if (ctx->argc > 1) {
        if (gs_string_to_uint32(ctx->argv[1], &tasks) != GS_OK) {
            return GS_ERROR_ARG;
        }
    }
    for (unsigned int task = 1; task <= tasks; ++task) {
        task_ctx_t * t = calloc(1, sizeof(*t));
        t->no = task;
        snprintf(t->name, sizeof(t->name), "%s-%u", name, t->no);
        gs_thread_create(t->name, func, t, TASK_STACK_SIZE, GS_THREAD_PRIORITY_LOW, 0, NULL);
    }
    return GS_OK;
}

static void * task_errno(void * arg)
{
    print_errno(stdout);
    gs_thread_exit(0);
}

static int cmd_task_errno(gs_command_context_t * ctx)
{
    gs_error_t error = spawn_tasks(ctx, 1, "task-errno", task_errno);
    if (error == GS_OK) {
        // let the task run/complete
        gs_time_sleep_ms(1000);
    }
    return error;
}

static int cmd_jump(gs_command_context_t * ctx)
{
    void *ptr;
    if (gs_string_to_pointer(ctx->argv[1], &ptr) != GS_OK) {
        return GS_ERROR_ARG;
    }
    void (*jumpptr)(void) = ptr;
    (jumpptr)();

    return GS_OK;
}

static int cmd_exit(gs_command_context_t * ctx)
{
    int32_t status;
    if (gs_string_to_int32(ctx->argv[1], &status) != GS_OK) {
        return GS_ERROR_ARG;
    }
    exit(status);

    return GS_OK;
}

static int cmd__exit(gs_command_context_t * ctx)
{
    int32_t status;
    if (gs_string_to_int32(ctx->argv[1], &status) != GS_OK) {
        return GS_ERROR_ARG;
    }
    _exit(status);

    return GS_OK;
}

static int cmd_getpid(gs_command_context_t * ctx)
{
    fprintf(ctx->out, "getpid: %d\r\n", getpid());
    return GS_OK;
}

static int cmd_write(gs_command_context_t * ctx)
{
    int32_t fd;
    if (gs_string_to_int32(ctx->argv[1], &fd) != GS_OK) {
        return GS_ERROR_ARG;
    }
    uint32_t bytes;
    if (gs_string_to_uint32(ctx->argv[2], &bytes) != GS_OK) {
        return GS_ERROR_ARG;
    }

    uint8_t data[bytes+1];
    memset(data, 'A', bytes+1);
    errno = 0;
    int res = write(fd, data, bytes);
    fprintf(ctx->out, "res: %d, errno: %d\r\n", res, errno);
    return GS_OK;
}

static int cmd_read(gs_command_context_t * ctx)
{
    int32_t fd;
    if (gs_string_to_int32(ctx->argv[1], &fd) != GS_OK) {
        return GS_ERROR_ARG;
    }
    uint32_t bytes;
    if (gs_string_to_uint32(ctx->argv[2], &bytes) != GS_OK) {
        return GS_ERROR_ARG;
    }

    uint8_t data[bytes+1];
    memset(data, 'A', bytes+1);
    errno = 0;
    int res = read(fd, data, bytes);
    fprintf(ctx->out, "res: %d, errno: %d\r\n", res, errno);
    return GS_OK;
}

static int cmd_print_float(gs_command_context_t * ctx)
{
    float f = 1.2;
    fprintf(ctx->out, "float: %f\r\n", f);
    double d = 1000.1234;
    fprintf(ctx->out, "double: %e\r\n", d);
    return GS_OK;
}

static int cmd_print_basic(gs_command_context_t * ctx)
{
    int a = 10;
    fprintf(ctx->out, "%d\r\n", a);
    unsigned int b = 11;
    fprintf(ctx->out, "%u\r\n", b);
    long c = 12;
    fprintf(ctx->out, "%ld\r\n", c);
    unsigned long d = 13;
    fprintf(ctx->out, "%lu\r\n", d);

    int8_t int8 = 14;
    fprintf(ctx->out, "%"PRId8"\r\n", int8);
    uint8_t uint8 = 15;
    fprintf(ctx->out, "%"PRIu8"\r\n", uint8);

    int16_t int16 = 16;
    fprintf(ctx->out, "%"PRId16"\r\n", int16);
    uint16_t uint16 = 17;
    fprintf(ctx->out, "%"PRIu16"\r\n", uint16);

    int32_t int32 = 18;
    fprintf(ctx->out, "%"PRId32"\r\n", int32);
    uint32_t uint32 = 19;
    fprintf(ctx->out, "%"PRIu32"\r\n", uint32);

    int64_t int64 = 20;
    fprintf(ctx->out, "%"PRId64"\r\n", int64);
    uint64_t uint64 = 21;
    fprintf(ctx->out, "%"PRIu64"\r\n", uint64);

    const char * str = "This is a string";
    fprintf(ctx->out, "%s\r\n", str);
    fprintf(ctx->out, "%-30.30s\r\n", str);
    fprintf(ctx->out, "%30.30s\r\n", str);

    return GS_OK;
}

static void * task_exit(void * arg)
{
    gs_thread_exit(NULL);
}

static int cmd_task_exit(gs_command_context_t * ctx)
{
    return spawn_tasks(ctx, 1, "task-exit", task_exit);
}

static void * task_snprintf(void * arg)
{
    // no sleep or other calls that may trigger a task switch
    const unsigned int TASK_NUM = (unsigned int) GS_TYPES_PTR2UINT(arg);
    log_info("%s %u: impure %p", __FUNCTION__, TASK_NUM, _impure_ptr);
    unsigned int count = 0;
    uint32_t ref_ms = gs_time_rel_ms();
    for (;;) {
        const size_t BUF_SIZE = 100;
        char * buf = malloc(BUF_SIZE);
        snprintf(buf, BUF_SIZE, "%f", (float)count);
        snprintf(buf, BUF_SIZE, "%u, %u, %u, %u", TASK_NUM, count, count, count);
        free(buf);
        ++count;
        uint32_t now_ms = gs_time_rel_ms();
        if (gs_time_diff_ms(ref_ms, now_ms) >= 10000) {
            ref_ms = now_ms;
            log_info("%s %u: count %u", __FUNCTION__, TASK_NUM, count);
        }
    }
    gs_thread_exit(NULL);
}

static int cmd_task_snprintf(gs_command_context_t * ctx)
{
    return spawn_tasks(ctx, 3, "task-snprintf", task_snprintf);
}

static void * task_fileio(void * arg)
{
    // no sleep or other calls that may trigger a task switch
    task_ctx_t * t = arg;

    log_info("%s %u: impure %p", __FUNCTION__, t->no, _impure_ptr);
    unsigned int count = 0;
    uint32_t ref_ms = gs_time_rel_ms();

    char filename[100];
    snprintf(filename, sizeof(filename), "/flash/%s-%u", __FUNCTION__, t->no);

    for (;;) {
        char buf[100];
        snprintf(buf, sizeof(buf), "%u, %u, %u, %u", t->no, count, count, count);
        FILE * fp = fopen(filename, "w");
        if (fp) {
            fprintf(fp, "%u, %u, %u, %u", t->no, count, count, count);
            fclose(fp);
            fp = fopen(filename, "r");
            if (fp) {
                char rbuf[100];
                const char * readbuf = fgets(rbuf, sizeof(rbuf), fp);
                if (readbuf) {
                    if (strcmp(readbuf, buf) != 0) {
                        log_error("Data differs [%s] != [%s], filename: %s", buf, readbuf, filename);
                    }
                } else {
                    log_error("Failed to read from file: %p/%s, errno: %d", fp, filename, errno);
                }
                fclose(fp);
            } else {
                log_error("Failed to open/read file: %s, errno: %d", filename, errno);
            }
        } else {
            log_error("Failed to open/create file: %s, errno: %d", filename, errno);
        }

        ++count;
        uint32_t now_ms = gs_time_rel_ms();
        if (gs_time_diff_ms(ref_ms, now_ms) >= 10000) {
            ref_ms = now_ms;
            log_info("%s %u: count %u", __FUNCTION__, t->no, count);
        }
    }
    gs_thread_exit(NULL);
}

static int cmd_task_fileio(gs_command_context_t * ctx)
{
    return spawn_tasks(ctx, 3, "task-fileio", task_fileio);
}

static int cmd_clock_get_nsec(gs_command_context_t * ctx)
{
    uint32_t ms = 0;
    if ((ctx->argc > 1) && (gs_string_to_uint32(ctx->argv[1], &ms) != GS_OK)) {
        return GS_ERROR_ARG;
    }

    const uint64_t t1_ns = gs_clock_get_nsec();
    gs_time_sleep_ms(ms);
    const uint64_t t2_ns = gs_clock_get_nsec();

    fprintf(ctx->out, "t1: %"PRIu64", t2: %"PRIu64", diff: %"PRIu64"\r\n",
            t1_ns, t2_ns, (t2_ns - t1_ns));

    return (t2_ns >= t1_ns) ? GS_OK : GS_ERROR_UNKNOWN; // wrapping isn't handled
}

static int cmd_delay_us(gs_command_context_t * ctx)
{
    static bool init = false;
    if (init == false) {
        init = true;
        gs_delay_init();
    }

    uint32_t us;
    if (gs_string_to_uint32(ctx->argv[1], &us) != GS_OK) {
        return GS_ERROR_ARG;
    }

    // Time gs_clock_get_nsec() function
    uint64_t start_ns = gs_clock_get_nsec();
    uint64_t stop_ns = gs_clock_get_nsec();
    if (stop_ns < start_ns) {
        return GS_ERROR_AGAIN;
    }
    const uint64_t overhead_ns = (stop_ns - start_ns);
    fprintf(ctx->out, "Time to call gs_clock_get_nsec() twice, %u uS\r\n", (unsigned int) (overhead_ns / 1000ULL));

    // Time actual 'delay_us'
    start_ns = gs_clock_get_nsec();
    gs_delay_us(us);
    stop_ns = gs_clock_get_nsec();
    if (stop_ns < start_ns) {
        return GS_ERROR_AGAIN;
    }
    uint64_t elapsed_us = ((stop_ns - start_ns - overhead_ns) / 1000ULL);
    fprintf(ctx->out, "Delay %"PRIu32" uS, elapsed: %"PRIu64" uS (total time: %"PRIu64" nS)\r\n", us, elapsed_us, (stop_ns - start_ns));

    return (elapsed_us >= us) ? GS_OK : GS_ERROR_UNKNOWN;
}

static int cmd_sleep_ms(gs_command_context_t * ctx)
{
    uint32_t ms;
    if (gs_string_to_uint32(ctx->argv[1], &ms) != GS_OK) {
        return GS_ERROR_ARG;
    }

    // Time gs_clock_get_nsec() function
    uint64_t start_ns = gs_clock_get_nsec();
    uint64_t stop_ns = gs_clock_get_nsec();
    if (stop_ns < start_ns) {
        return GS_ERROR_AGAIN;
    }
    const uint64_t overhead_ns = (stop_ns - start_ns);
    fprintf(ctx->out, "Time to call gs_clock_get_nsec() twice, %u uS\r\n", (unsigned int) (overhead_ns / 1000ULL));

    // Time actual 'sleep mS'
    start_ns = gs_clock_get_nsec();
    gs_time_sleep_ms(ms);
    stop_ns = gs_clock_get_nsec();
    if (stop_ns < start_ns) {
        return GS_ERROR_AGAIN;
    }
    uint64_t elapsed_us = ((stop_ns - start_ns - overhead_ns) / 1000ULL);
    fprintf(ctx->out, "Sleep %"PRIu32" mS, elapsed: %"PRIu64" uS (%"PRIu64" nS)\r\n", ms, elapsed_us, (stop_ns - start_ns));

    return (elapsed_us >= (ms * 1000ULL)) ? GS_OK : GS_ERROR_UNKNOWN;
}

static const gs_command_t cmd_syscalls_sub[] = {
    {
        .name="getpid",
        .help="Calls getpid()",
        .mandatory_args=GS_COMMAND_NO_ARGS,
        .handler=cmd_getpid,
    },{
        .name="write",
        .help="Calls write()",
        .usage="<fd> <bytes>",
        .mandatory_args=2,
        .handler=cmd_write,
    },{
        .name="read",
        .help="Calls read()",
        .usage="<fd> <bytes>",
        .mandatory_args=2,
        .handler=cmd_read,
    },{
        .name="exit",
        .help="Calls exit() -> reset()",
        .usage="<status>",
        .mandatory_args=1,
        .handler=cmd_exit,
    },{
        .name="_exit",
        .help="Calls _exit() -> reset()",
        .usage="<status>",
        .mandatory_args=1,
        .handler=cmd__exit,
    }
};

static const gs_command_t cmd_test_sub[] = {
    {
        .name = "info",
        .help = "Show general info",
        .handler = cmd_info,
        .mandatory_args=GS_COMMAND_NO_ARGS,
    },{
        .name = "errno",
        .help = "Show errno",
        .usage = "[errno]",
        .handler = cmd_errno,
        .optional_args=1,
    },{
        .name="malloc",
        .help="Malloc memory",
        .usage="<bytes>",
        .mandatory_args=1,
        .handler=cmd_malloc,
    },{
        .name="free",
        .help="Free memory",
        .usage="<pointer>",
        .mandatory_args=1,
        .handler=cmd_free,
    },{
        .name="jump",
        .help="Jump to address",
        .usage="<address>",
        .mandatory_args=1,
        .handler=cmd_jump,
    },{
        .name="print_float",
        .help="Print floats",
        .mandatory_args=GS_COMMAND_NO_ARGS,
        .handler=cmd_print_float,
    },{
        .name="print_basic",
        .help="Print integers and string types",
        .mandatory_args=GS_COMMAND_NO_ARGS,
        .handler=cmd_print_basic,
    },{
        .name="task-exit",
        .help="Create task that terminates",
        .usage="[tasks]",
        .optional_args=1,
        .handler=cmd_task_exit,
    },{
        .name="task-errno",
        .help="Create task and print errno",
        .usage="[tasks]",
        .optional_args=1,
        .handler=cmd_task_errno,
    },{
        .name="task-snprintf",
        .help="Create 3 tasks cont. doing snprintf & malloc",
        .usage="[tasks]",
        .optional_args=1,
        .handler=cmd_task_snprintf,
    },{
        .name="task-fileio",
        .help="Create 3 tasks cont. doing file I/O",
        .usage="[tasks]",
        .optional_args=1,
        .handler=cmd_task_fileio,
    },{
        .name="file-test",
        .help="Runs a number of file operations",
        .usage="<file-path-prefix>",
        .mandatory_args=1,
        .handler=cmd_file_test,
    },{
        .name="gs_clock_get_nsec",
        .help="Show nsec",
        .usage="[sleep mS]",
        .optional_args=1,
        .handler=cmd_clock_get_nsec,
    },{
        .name="delay",
        .help="delay uS",
        .usage="<us>",
        .mandatory_args=1,
        .handler=cmd_delay_us,
    },{
        .name="sleep",
        .help="sleep",
        .usage="<mS>",
        .mandatory_args=1,
        .handler=cmd_sleep_ms,
    },{
        .name="syscalls",
        .help="System calls",
        .chain = GS_COMMAND_INIT_CHAIN(cmd_syscalls_sub),
    },
};

static const gs_command_t GS_COMMAND_ROOT cmd_test[] = {
    {
        .name = "embed-test",
        .help = "Test commands",
        .chain = GS_COMMAND_INIT_CHAIN(cmd_test_sub),
    }
};

gs_error_t gs_embed_register_test_commands(void)
{
    return GS_COMMAND_REGISTER(cmd_test);
}
