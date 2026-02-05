/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/ftp/server.h>

#include <string.h>
#include <malloc.h>

#include <gs/util/crc32.h>
#include <gs/util/zip/zip.h>
#include <csp/csp_endian.h>

#define UPLOAD_ADDR 0x50000000
#define UPLOAD_SIZE 0x00100000

static uint32_t file_mem_addr;
static uint32_t file_size;
static uint32_t file_chunk_size;
static uint32_t file_chunks;

static unsigned char * ram_map = NULL;

static gs_ftp_return_t ftp_ram_get_crc(void * state, uint32_t * crc_arg)
{
    if (!crc_arg)
        return GS_FTP_RET_INVAL;

    *crc_arg = gs_crc32(GS_TYPES_UINT2PTR(file_mem_addr), file_size);

    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_ram_upload(void * state, char const * path, uint32_t memaddr, uint32_t size, uint32_t chunk_size)
{
    file_mem_addr = memaddr;
    file_size = size;
    file_chunk_size = chunk_size;
    file_chunks = (size + chunk_size - 1) / chunk_size;

    /* Allocate map */
    ram_map = malloc(file_chunks);
    if (ram_map == NULL)
        return GS_FTP_RET_NOSPC;

    memset(ram_map, 0, file_chunks);

    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_ram_download(void * state, char const * path, uint32_t memaddr, uint32_t memsize, uint32_t chunk_size, uint32_t * size, uint32_t * crc)
{
    file_mem_addr = memaddr;
    file_size = memsize;
    file_chunk_size = chunk_size;
    file_chunks = (memsize + chunk_size - 1) / chunk_size;

    *size = memsize;

    /* Calculate CRC */
    return ftp_ram_get_crc(state, crc);
}

static gs_ftp_return_t ftp_ram_write_chunk(void * state, uint32_t chunk, uint8_t * src, uint32_t size)
{
    /* Write to mem */
    void * dst = (void *) (uintptr_t) (file_mem_addr + chunk * file_chunk_size);

    memcpy(dst, src, size);

    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_ram_read_chunk(void * state, uint32_t chunk, uint8_t * dst, uint32_t size)
{
    /* Read from mem */
    void * src = (void *) (uintptr_t) (file_mem_addr + chunk * file_chunk_size);

    memcpy(dst, src, size);

    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_ram_get_status(void * state, uint32_t chunk, int * status)
{
    if (status) {
        *status = ram_map[chunk];
        return GS_FTP_RET_OK;
    } else {
        return GS_FTP_RET_INVAL;
    }
}

static gs_ftp_return_t ftp_ram_set_status(void * state, uint32_t chunk)
{
    /* Write to map */
    ram_map[chunk] = 1;

    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_ram_abort(void * state)
{
    free(ram_map);
    ram_map = NULL;
    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_ram_done(void * state)
{
#if defined __arm__ && !defined __linux__
    gs_zip_decompress_data(file_mem_addr, UPLOAD_SIZE, (void *)(UPLOAD_ADDR), UPLOAD_SIZE);
#endif

    return ftp_ram_abort(NULL);
}

static const gs_ftp_backend_t backend_ram = {
    .upload 		= ftp_ram_upload,
    .download		= ftp_ram_download,
    .chunk_write	= ftp_ram_write_chunk,
    .chunk_read 	= ftp_ram_read_chunk,
    .status_get 	= ftp_ram_get_status,
    .status_set 	= ftp_ram_set_status,
    .crc 		= ftp_ram_get_crc,
    .abort 		= ftp_ram_abort,
    .done 		= ftp_ram_done,
    .timeout		= ftp_ram_abort,
};

const gs_ftp_backend_t * gs_ftp_server_backend_ram(void)
{
    return &backend_ram;
}
