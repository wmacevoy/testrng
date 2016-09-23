#include <string.h>
#include <stdlib.h>

#include "rng_reader.h"

reader_t *rng_reader(const char *config) {
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
