/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/embed/unistd.h>
#include <gs/util/check.h>

// Holds current working directory.
static char gs_embed_cwd[50];

gs_error_t gs_setcwd(const char * cwd)
{
    GS_CHECK_ARG(gs_string_empty(cwd) == false);

    if (strnlen(cwd, sizeof(gs_embed_cwd)) >= sizeof(gs_embed_cwd)) {
        return GS_ERROR_RANGE;
    }

    strcpy(gs_embed_cwd, cwd);
    return GS_OK;
}

gs_error_t gs_getcwd(char * buf, size_t bufsize)
{
    GS_CHECK_ARG((buf != NULL) && bufsize);

    if (gs_string_empty(gs_embed_cwd)) {
        return GS_ERROR_NOT_FOUND;
    }

    if (strnlen(gs_embed_cwd, sizeof(gs_embed_cwd)) >= bufsize) {
        return GS_ERROR_RANGE;
    }

    strcpy(buf, gs_embed_cwd);
    return GS_OK;
}
