#include "load.h"
#include "rng_load.h"

reader_t *rng_load(const char *config) {
  return (reader_t*)load("rng",config);
}
