#include "stats_load.h"
#include "load.h"

stats_t *stats_load(const char *config) {
  return (stats_t*)load("stats",config);
}
