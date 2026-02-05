/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <beacon_service/beacon_file.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#include <gs/util/clock.h>
#include <gs/util/log.h>
#include <gs/util/byteorder.h>

#define HK_FILE_MAGIC   0x686b0001 /* 'h' 'k' 0x0001 */
#define HK_BEACON_MAGIC 0x62630001 /* 'b' 'c' 0x0001 */

#define BYTEORDER_BE 1
#define BYTEORDER_LE 2

static uint8_t bcn_file_byte_order = BYTEORDER_BE;

FILE * hk_bcn_file_open(const char *filename, int append)
{

    FILE *fp = NULL;
    if (filename && strlen(filename)) {
        uint32_t fsize = 0;
        struct stat statbuf;
        if (stat(filename, &statbuf) == 0) {
            fsize = (uint32_t) statbuf.st_size;
        }

        if (!append || (fsize == 0)) {
            log_info("Creating %s", filename);
            fp = fopen(filename, "w");
            if (fp == NULL) {
                log_error("Error opening %s", filename);
            } else {
                uint32_t magic = HK_BEACON_MAGIC;
                fwrite(&magic, 1, sizeof(uint32_t), fp);
            }

        } else {
            log_info("Appending to %s", filename);
            fp = fopen(filename, "a+");
            if (fp == NULL) {
                log_error("Error opening %s for appending", filename);
            } else {
                /* Do nothing */
            }
        }
    }

    return fp;
}

int hk_bcn_file_close(FILE *fp)
{

    if (fp) {
        fclose(fp);
        return 0;
    }

    return -1;
}

int hk_bcn_file_append(FILE *fp, uint8_t type, uint16_t length, void * data)
{

    if (fp) {
        /* Write data header: length, type */
        fwrite(&length, 1, sizeof(uint16_t), fp);
        fwrite(&type, 1, sizeof(uint8_t), fp);
        /* Write data */
        fwrite(data, 1, length, fp);
        return 0;
    }

    return -1;
}

FILE * hk_bcn_file_open_read(const char *filename)
{

    FILE *fp = NULL;
    if (filename && strlen(filename)) {
        uint32_t fsize = 0;
        struct stat statbuf;
        if (stat(filename, &statbuf) == 0) {
            fsize = (uint32_t) statbuf.st_size;
        }
        fp = fopen(filename, "r");
        if (fp == NULL) {
            log_error("Error opening %s", filename);
        } else {
            uint32_t magic = 0;
            if (fsize <= sizeof(magic)) {
                log_error("Beacon file %s is too small or empty (%"PRIu32" bytes)", filename, fsize);
                fclose(fp);
                return NULL;
            }
            if (!fread(&magic, 1, sizeof(uint32_t), fp)) {
                log_error("Error reading from file %s", filename);
                fclose(fp);
                return NULL;
            }
            if (magic == util_betoh32(HK_BEACON_MAGIC)) {
                log_info("Beacon file BE");
                bcn_file_byte_order = BYTEORDER_BE;
            } else if (magic == util_letoh32(HK_BEACON_MAGIC)) {
                log_info("Beacon file LE");
                bcn_file_byte_order = BYTEORDER_LE;
            } else {
                log_error("Wrong magic file id for beacon file %s: %"PRIX32"", filename, magic);
                fclose(fp);
                return NULL;
            }
            log_info("Opened beacon file %s (%"PRIu32" bytes)", filename, fsize);
        }
    }

    return fp;
}

int hk_bcn_file_read(FILE *fp, uint8_t* type, uint16_t* length, void* data,
                     uint16_t max_length)
{

    if (fp) {
        /* Read data header: length, type */
        if (fread(length, 1, sizeof(uint16_t), fp)
            && fread(type, 1, sizeof(uint8_t), fp)) {
            if (bcn_file_byte_order == BYTEORDER_BE) {
                *length = util_betoh16(*length);
            } else if (bcn_file_byte_order == BYTEORDER_LE) {
                *length = util_letoh16(*length);
            } else {
                return -1;
            }
            if (*length <= max_length) {
                /* Read data */
                if (fread(data, 1, *length, fp)) {
                    log_debug("Read beacon: type %u length %"PRIu16"", 
                              *type, *length);
                    return 0;
                } else {
                    log_error("Error reading beacon data");
                }
            } else {
                log_error("Data length too large, length: %u max_length: %u", *length, max_length);
            }
        } else {
            //log_error("Error reading beacon header");
        }
    }

    return -1;
}
