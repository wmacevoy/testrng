#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "stats_load.h"
#include "rng_load.h"
#include <mcheck.h>

int main(int argc, char *argv[])
{
  if (getenv("MALLOC_TRACE") != 0) {
    printf("memory trace on.\n");
    mtrace();
  }
  int argi;
  char *rng_config = 0;
  char *stats_config = 0;
  reader_t *rng  = 0;
  stats_t *stats  = 0;

  for (argi=1; argi<argc; ++argi) {
    if (strcmp(argv[argi],"--rng")==0) {
      rng_config=argv[++argi];
      continue;
    }
    if (strcmp(argv[argi],"--stats")==0) {
      stats_config = argv[++argi];
      continue;
    }
    printf("unknown arg '%s'\n",argv[argi]);
    exit(1);
  }

  if (rng_config == 0) rng_config = "reader /dev/urandom";
  if (stats_config == 0) stats_config = "max64";
   
  rng = rng_load(rng_config);
  if (rng == 0) {
    printf("could not open rng config: %s\n", rng_config);
    exit (1);
  }
  stats = stats_load(stats_config);
  if (stats == 0) {
    printf("could not open stats config: %s\n", stats_config);
    exit (1);
  }

  stats_run(stats,rng);
  char *name = stats_get_string_by_name(stats,"name");
  char *config = stats_get_string_by_name(stats,"config");
  char *results = stats_get_string_by_name(stats,"results");
  printf("%s --rng \"%s\" --stats \"%s %s\"\n",
         argv[0],rng_config,name,config);
  printf("> %s\n",results);

  free(name);
  free(config);
  free(results);

  return 0;
}
