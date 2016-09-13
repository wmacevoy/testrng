#pragma once

#include "stats.h"

#ifdef __cplusplus
extern "C" {
#endif

stats_t *stats_load(const char *command);

#ifdef __cplusplus
} /* extern "C" */
#endif
