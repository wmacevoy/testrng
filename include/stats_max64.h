#pragma once

#include "stats.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STATS_MAX64_EXPORT
#define STATS_MAX64_EXPORT __declspec(dllimport)
#endif

  stats_t* stats_max64(const char *config);

#ifdef __cplusplus
} /* extern "C" */
#endif
