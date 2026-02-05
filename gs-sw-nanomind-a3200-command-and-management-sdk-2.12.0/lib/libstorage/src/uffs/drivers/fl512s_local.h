/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/thirdparty/flash/spn_fl512s.h>

// Name of partitions.
#define GS_FL512S_NAME_PARTITION_0  "flash0"
#define GS_FL512S_NAME_PARTITION_1  "flash1"

// Mount file-system
gs_error_t gs_fl512s_uffs_mount(unsigned int partition);
