#pragma once

#include "reader.h"

#ifdef __cplusplus
extern "C" {
#endif

reader_t *rng_rdrand(const char *config);

#ifdef __cplusplus
} /* extern "C" */
#endif
