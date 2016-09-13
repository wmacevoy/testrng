#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "reader.h"
#include "rng_rdrand.h"
#include "stats_load.h"

reader_t *rng_load(char *config) {
  if (strcmp(config,"rng_rdrand") == 0) {
    return rng_rdrand();
  }

  if (strcmp(config,"/dev/stdin") == 0) {
    return dev_reader(0);
  }

  if (strlen(config) > 0 && config[strlen(config)-1]=='|') {
    char *cmd = strdup(config);
    cmd[strlen(cmd)-1]=0;
    reader_t *r = pipe_reader(cmd);
    free(cmd);
    return r;
  }

  return file_reader(fopen(config,"rb"));
}

int main(int argc, char *argv[])
{
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

  if (rng_config == 0) rng_config = "/dev/urandom";
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
  char *name = stats_get_string(stats,
                                stats_get_string_index(stats,"name"));
  char *summary = stats_get_string(stats,
                                   stats_get_string_index(stats,"summary"));

  printf("%s[%s](%s)=%s\n",name,stats_config,rng_config,summary);
  free(name);
  free(summary);

  return 0;
}
