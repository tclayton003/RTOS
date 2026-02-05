/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include "vfs_local.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <gs/embed/drivers/sys/reset.h>
#include <gs/util/gosh/command.h>
#include <gs/util/string.h>
#include <gs/util/crc32.h>
#include <gs/util/hexdump.h>
#include <gs/util/log.h>
#include <gs/util/time.h>

static int execute_fs_test(gs_command_context_t *ctx, const char * path, int fd, char * data_w, char * data_r, int size)
{
    /* Create test pattern */
    for (int i = 0; i < size; i++) {
        data_w[i] = i & 0xff;
    }

    // Write
    uint32_t start_time = gs_time_rel_ms();
    int bytes = write(fd, data_w, size);
    if (bytes != size) {
        fprintf(ctx->out, "Failed to write test data to %s (wrote %d bytes)\r\n", path, bytes);
        return GS_ERROR_IO;
    }
    fsync(fd);
    unsigned int ms = gs_time_diff_ms(start_time, gs_time_rel_ms());
    fprintf(ctx->out, "Wrote %d bytes in %u mS (%u KBytes/sec)\r\n", size, ms, size / ((ms < 1) ? 1 : ms));

    /* Go to beginning */
    lseek(fd, 0, SEEK_SET);

    // Read back
    start_time = gs_time_rel_ms();
    bytes = read(fd, data_r, size);
    if (bytes != size) {
        fprintf(ctx->out, "Failed to read test data from %s (read %d bytes)\r\n", path, bytes);
        return GS_ERROR_IO;
    }
    fsync(fd);
    ms = gs_time_diff_ms(start_time, gs_time_rel_ms());
    fprintf(ctx->out, "Read %d bytes in %u mS (%u KBytes/sec)\r\n", size, ms, size / ((ms < 1) ? 1 : ms));

    // Verify data
    if (memcmp(data_w, data_r, size) != 0) {
        fprintf(ctx->out, "Failed to verify data (read != written)!\r\n");
        return GS_ERROR_DATA;
    }

    return GS_OK;
}

static int cmd_fs_test(gs_command_context_t *ctx)
{
    const char * path = ctx->argv[1];

    int32_t size = 100000;
    if (ctx->argc > 2) {
        if (gs_string_to_int32(ctx->argv[2], &size)) {
            return GS_ERROR_ARG;
        }
    }

    int fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0666);
    if (fd < 0) {
        fprintf(ctx->out, "Failed to create file: [%s]\r\n", path);
        return GS_ERROR_IO;
    }

    gs_error_t error;
    char * data_w = malloc(size);
    char * data_r = malloc(size);
    if (data_w && data_r) {
        error = execute_fs_test(ctx, path, fd, data_w, data_r, size);
    } else {
        fprintf(ctx->out, "Failed to allocate memory buffer, size %"PRId32" \r\n", size);
        error = GS_ERROR_ALLOC;
    }

    free(data_r);
    free(data_w);
    close(fd);

    return error;
}

static int objects_under(const char *dir)
{
    int count = 0;
    struct dirent *ent;

    DIR *dirp = opendir(dir);
    if (dirp) {
        // cppcheck-suppress readdirCalled - readdir_r not supported on avr32
        while ((ent = readdir(dirp)) != NULL) {
            if (ent->d_name[0] != '.')
                count++;
        }
        closedir(dirp);
    }

    return count;
}

static int cmd_fs_ls(gs_command_context_t *ctx)
{
    const char * path = ctx->argv[1];
    DIR * dirp = opendir(path);
    if (dirp == NULL) {
        return GS_ERROR_NOT_FOUND;
    }

    /* Loop through directories */
    struct dirent *ent;
    // cppcheck-suppress readdirCalled - readdir_r not supported
    while ((ent = readdir(dirp))) {

        if (ent->d_name[0] == '.') {
            continue;
        }

        char buf[MAX_FILENAME_LENGTH+2]; // add space for /
        GS_STRNCPY(buf, path);
        char * sub = buf;
        if (path[strlen(path)-1] != '/') {
            sub = strcat(buf, "/");
        }
        sub = strcat(sub, ent->d_name);
        char bytebuf[25];
        if (ent->d_type & DT_DIR) {
            snprintf(bytebuf, sizeof(bytebuf), "%d", objects_under(sub));
            strcat(ent->d_name, "/");
        } else {
            struct stat stat_buf;
            stat(sub, &stat_buf);
            gs_string_bytesize(stat_buf.st_size, bytebuf, sizeof(bytebuf));
        }

        /* Name */
        fprintf(ctx->out, "%-15s %6s\r\n", ent->d_name, bytebuf);
    }

    closedir(dirp);

    return GS_OK;
}

static int cmd_fs_mkdir(gs_command_context_t *ctx)
{
    const char * path = ctx->argv[1];
    return (mkdir(path, 0666) == 0) ? GS_OK : GS_ERROR_IO;
}

static int cmd_fs_rm(gs_command_context_t *ctx)
{
    const char * path = ctx->argv[1];

    bool recursive = false;
    if (ctx->argc > 2) {
        if (gs_string_to_bool(ctx->argv[2], &recursive)) {
            return GS_ERROR_ARG;
        }
    }

    struct stat st;
    if (stat(path, &st) < 0) {
        return GS_ERROR_NOT_FOUND;
    }

    if (st.st_mode & S_IFDIR) {
        return (vfs_rmdir_recursive(path, recursive) == 0) ? GS_OK : GS_ERROR_IO;
    }
    return (remove(path) == 0) ? GS_OK : GS_ERROR_IO;

}

static int cmd_fs_mv(gs_command_context_t *ctx)
{
    const char * old = ctx->argv[1];
    const char * new = ctx->argv[2];
    return (rename(old, new) == 0) ? GS_OK : GS_ERROR_IO;
}

static int cmd_fs_cp(gs_command_context_t *ctx)
{
    const char * old = ctx->argv[1];
    const char * new = ctx->argv[2];

    int fdold = open(old, O_RDONLY, 0666);
    if (fdold < 0) {
        fprintf(ctx->out, "cp: failed to open [%s]\r\n", old);
        return GS_ERROR_NOT_FOUND;
    }

    int fdnew = open(new, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (fdnew < 0) {
        fprintf(ctx->out, "cp: failed to open [%s]\r\n", new);
        close(fdold);
        return GS_ERROR_IO;
    }

    int in;
    do {
        char buf[512];

        in = read(fdold, buf, sizeof(buf));
        if (in > 0) {
            int out = write(fdnew, buf, in);
            if (in != out) {
                fprintf(ctx->out, "cp: failed to write to %s\r\n", new);
                close(fdold);
                close(fdnew);
                return GS_ERROR_IO;
            }
        }
    } while (in > 1);

    close(fdold);
    close(fdnew);

    return GS_OK;
}

static int cmd_fs_touch(gs_command_context_t *ctx)
{
    const char * path = ctx->argv[1];
    int fd = open(path, O_CREAT | O_WRONLY, 0666);
    if (fd < 0) {
        return GS_ERROR_IO;
    }

    close(fd);
    return GS_OK;
}

static int cmd_fs_cat(gs_command_context_t *ctx)
{
    const char * path = ctx->argv[1];
    int fd = open(path, O_RDONLY, 0666);
    if (fd < 0) {
        return GS_ERROR_NOT_FOUND;
    }

    char buf;
    while(read(fd, &buf, 1) > 0) {
        fprintf(ctx->out, "%c", buf);
    }
    close(fd);

    if (buf != '\n' && buf != '\r') {
        fprintf(ctx->out, "\r\n");
    }

    return GS_OK;
}

static inline int output_chars(int fd, int ch)
{
    gs_stdio_putchar(ch);
    const char d = ch;
    return write(fd, &d, sizeof(d));
}

static gs_error_t cmd_fs_append_read_write(gs_command_context_t *ctx, int fd)
{
    fprintf(ctx->out, "Minimalist file editor to append chars to a file.\r\nHit ctrl+e to end and save file.\r\n");

    // Wait for ^q to quit.
    for (;;) {

        /* Get character */
        int ch;
        gs_error_t error = gs_command_wait_key(ctx, &ch, -1);
        if (error) {
            return error;
        }

        switch (ch) {

            /* CTRL + q */
            case 0x11:
            /* CTRL + e */
            case 0x05:
                return GS_OK;

            default:
                if (output_chars(fd, ch) < 0) {
                    return GS_ERROR_IO;
                }
                if (ch == '\r') {
                    if (output_chars(fd, '\n') < 0) {
                        return GS_ERROR_IO;
                    }
                }
                break;
        }
    }
}

static int cmd_fs_append(gs_command_context_t *ctx)
{
    const char * path = ctx->argv[1];

    /* Open file */
    int fd = open(path, O_CREAT | O_APPEND | O_WRONLY, 0666);
    if (fd < 0) {
        return GS_ERROR_IO;
    }

    gs_error_t error = cmd_fs_append_read_write(ctx, fd);
    close(fd);

    return error;
}

static int cmd_fs_chksum(gs_command_context_t *ctx)
{
    const char * path = ctx->argv[1];

    FILE *fp = fopen(path, "r");
    if (!fp) {
        return GS_ERROR_NOT_FOUND;
    }

    /* Calculate CRC32 */
    char buf[100];
    uint32_t crc = gs_crc32_init();
    size_t bytes;
    while((bytes = fread(buf, 1, sizeof(buf), fp)) > 0) {
        crc = gs_crc32_update(crc, buf, bytes);
    }
    crc = gs_crc32_finalize(crc);

    fprintf(ctx->out, "Checksum: 0x%" PRIx32 "\r\n", crc);

    fclose(fp);

    return GS_OK;
}

static int cmd_fs_hexdump(gs_command_context_t *ctx)
{
    const char * path = ctx->argv[1];

    /* Read file size */
    struct stat stat_buf;
    if (stat(path, &stat_buf) != 0) {
        return GS_ERROR_NOT_FOUND;
    }

    /* Open file */
    FILE * fp = fopen(path, "r");
    if (!fp) {
        return GS_ERROR_NOT_FOUND;
    }

    size_t size = stat_buf.st_size;

    /* Allocate buffer */
    gs_error_t error;
    char * buf = malloc(size);
    if (buf) {
        /* Read file */
        if (fread(buf, 1, size, fp) == size) {
            gs_hexdump_addr(buf, size, 0);
            error = GS_OK;
        } else {
            fprintf(ctx->out, "hexdump: failed to read %zu bytes from %s\r\n", size, path);
            error = GS_ERROR_IO;
        }
    } else {
        fprintf(ctx->out, "hexdump: cannot allocate %zu bytes memory for %s\r\n", size, path);
        error = GS_ERROR_ALLOC;
    }

    free(buf);
    fclose(fp);

    return error;
}

#if (GS_STORAGE_VFS)
static int cmd_fs_mkfs(gs_command_context_t *ctx)
{
    const char * path = ctx->argv[1];

    bool force = false;
    if (ctx->argc > 2) {
        if (gs_string_to_bool(ctx->argv[2], &force)) {
            return GS_ERROR_ARG;
        }
    }
    const unsigned int flags = force ? VFS_MKFS_FORCE : 0;

    int ret = vfs_mkfs(path, flags);
    if (ret) {
        fprintf(ctx->out, "failed to format: [%s]\r\n", strerror(-ret));
        return GS_ERROR_IO;
    }

    if (flags & VFS_MKFS_FORCE) {
        log_warning("File system created, resetting now....");
        gs_sys_reset(GS_SYS_RESET_FILE_SYSTEM);
    }

    return GS_OK;
}

static void part_cb(const char *path, void * _ctx)
{
    gs_command_context_t * ctx = _ctx;
    
    struct vfs_statfs statfs;
    memset(&statfs, 0, sizeof(statfs));
    if (vfs_statfs(path, &statfs) < 0) {
        return;
    }

    char sizebuf[30], usebuf[30], availbuf[30];
    gs_string_bytesize(statfs.total, sizebuf, sizeof(sizebuf));
    gs_string_bytesize(statfs.used, usebuf, sizeof(usebuf));
    gs_string_bytesize(statfs.free, availbuf, sizeof(availbuf));

    char original_fs[MAX_FILENAME_LENGTH + 5] = {0};
    if (statfs.true_name) {
        strncat(original_fs, " -> ", 4);
        strncat(original_fs, statfs.true_name, MAX_FILENAME_LENGTH);
    }

    fprintf(ctx->out, "%-11s %7s %7s %7s %4u%% /%s%s\r\n",
            statfs.name, sizebuf, usebuf, availbuf,
            (unsigned)((statfs.used * (uint64_t)100) / statfs.total), path, original_fs);
}

static int cmd_fs_df(gs_command_context_t *ctx)
{
    fprintf(ctx->out, "File system    Size    Used   Avail  Use%% Mounted on\r\n");

    vfs_itertate_partitions(part_cb, ctx);

    return GS_OK;
}
#endif

static const gs_command_t GS_COMMAND_ROOT fs_commands[] = {
    {
        .name = "testfs",
        .help = "fs: Test R/W speeds",
        .handler = cmd_fs_test,
        .usage = "<file> [size]",
        .mandatory_args = 1,
        .optional_args = 1,
    },{
        .name = "ls",
        .help = "fs: List files in directory",
        .handler = cmd_fs_ls,
        .usage = "<dir>",
        .mandatory_args = 1,
    },{
        .name = "rm",
        .help = "fs: Remove path (file or dir)",
        .handler = cmd_fs_rm,
        .usage = "<path> [recursive]",
        .mandatory_args = 1,
        .optional_args = 1,
    },{
        .name = "mv",
        .help = "fs: Move (rename) file",
        .handler = cmd_fs_mv,
        .usage = "<from> <to>",
        .mandatory_args = 2,
    },{
        .name = "cp",
        .help = "fs: Copy file",
        .handler = cmd_fs_cp,
        .usage = "<source> <dest>",
        .mandatory_args = 2,
    },{
        .name = "mkdir",
        .help = "fs: Make directory",
        .handler = cmd_fs_mkdir,
        .usage = "<dir>",
        .mandatory_args = 1,
    },{
        .name = "touch",
        .help = "fs: Touch (create) file",
        .handler = cmd_fs_touch,
        .usage = "<file>",
        .mandatory_args = 1,
    },{
        .name = "cat",
        .help = "fs: Show content",
        .handler = cmd_fs_cat,
        .usage = "<file>",
        .mandatory_args = 1,
    },{
        .name = "append",
        .help = "fs: Append data",
        .handler = cmd_fs_append,
        .usage = "<file>",
        .mandatory_args = 1,
    },{
        .name = "chksum",
        .help = "fs: Calculate CRC32 checksum",
        .handler = cmd_fs_chksum,
        .usage = "<path>",
        .mandatory_args = 1,
    },{
        .name = "hexdump",
        .help = "fs: Hexdump",
        .handler = cmd_fs_hexdump,
        .usage = "<path>",
        .mandatory_args = 1,
#if (GS_STORAGE_VFS)
    },{
        .name = "mkfs",
        .help = "fs: build filesystem",
        .handler = cmd_fs_mkfs,
        .usage = "<partition> [force]",
        .mandatory_args = 1,
        .optional_args = 1,
    },{
        .name = "df",
        .help = "fs: show filesystem usage",
        .handler = cmd_fs_df,
        .mandatory_args = GS_COMMAND_NO_ARGS,
#endif
    },
};

gs_error_t vfs_register_commands(void)
{
    return GS_COMMAND_REGISTER(fs_commands);
}
