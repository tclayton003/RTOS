/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include "../local.h"

static gs_error_t dmc3_init(void)
{
    return GS_OK;
}

const dock_t dmc3_dock = {
    .name = "dmc3",
    .type = GS_A3200DOCK_DMC3,
    .init = dmc3_init,
};
