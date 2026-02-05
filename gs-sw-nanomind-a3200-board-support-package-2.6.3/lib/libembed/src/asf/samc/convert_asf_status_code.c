/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/embed/asf/samc/convert_asf_status_code.h>

gs_error_t gs_convert_asf_status_code(enum status_code status)
{
    switch (status) {
        case STATUS_OK: return GS_OK;
        case STATUS_BUSY: return GS_ERROR_BUSY;
        case STATUS_ERR_IO: return GS_ERROR_IO;
        case STATUS_ERR_TIMEOUT: return GS_ERROR_TIMEOUT;
        case STATUS_ERR_BAD_DATA: return GS_ERROR_DATA;
        case STATUS_ERR_NOT_FOUND: return GS_ERROR_NOT_FOUND;
        case STATUS_ERR_UNSUPPORTED_DEV: return GS_ERROR_NOT_SUPPORTED;
        case STATUS_ERR_NO_MEMORY: return GS_ERROR_ALLOC;
        case STATUS_ERR_INVALID_ARG: return GS_ERROR_ARG;
        case STATUS_ERR_BAD_FORMAT: return GS_ERROR_DATA;
        case STATUS_ERR_DENIED: return GS_ERROR_ACCESS;
        default: return GS_ERROR_IO;
    }
}
