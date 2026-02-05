#ifndef SRC_HOST_LOG_H
#define SRC_HOST_LOG_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include <gs/util/log.h>

GS_LOG_GROUP_EXTERN(gs_param_log);

// change default log group to my own
#define LOG_DEFAULT gs_param_log

#endif
