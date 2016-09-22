#include "stats_load.h"
#include "path_to_self.h"

#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

char *strcatdup(char *s, char *suffix) {
  int len0=strlen(s);
  int len1=strlen(suffix);
  int len=len0+len1;
  s=(char*) realloc(s,len+1);
  memcpy(s+len0,suffix,len1+1);
  return s;
}

static char *lib_dir = 0;
static char *get_lib_dir() {
  if (lib_dir == 0) {
    char *tmp = path_to_self();
    char *s = strrchr(tmp,'/');
    if (s != 0) *s = 0;
    s = strrchr(tmp,'/');
    if (s != 0) *s = 0;
    int tmplen = strlen(tmp);
    char *_lib_dir = (char*) malloc(tmplen+5);
    strcpy(_lib_dir,tmp);
    strcpy(_lib_dir+tmplen,"/lib");
    lib_dir=_lib_dir;
    free(tmp);
  }
  return lib_dir;
}

typedef stats_t *loader_t(const char *config);

stats_t *stats_load(const char *config) {
  char name[4096];
  int rest = -1;

  sscanf(config," %s %n",name,&rest);
  if (rest < 0) { 
    printf("no name cfg=%s\n",config);
    return 0;  
  } else {
    config += rest;
  }

  char lib[4096],sym[4096];
  snprintf(lib,sizeof(lib),"%s/libstats_%s.so",get_lib_dir(),name);
  snprintf(sym,sizeof(sym),"stats_%s",name);

  void *dl = dlopen(lib, RTLD_LAZY);

  loader_t *loader = (loader_t*) dlsym(dl,sym);
  if (loader != 0) {
    return loader(config);
  } else {
    return 0;
  }
}
