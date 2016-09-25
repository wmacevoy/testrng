#include "loader.h"
#include "path_to_self.h"

#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

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

typedef void *builder_t(const char *config);

void *load(const char *base, const char *config) {
  char name[4096];
  char lib[4096];
  char sym[4096];

  int rest = -1;

  sscanf(config," %s %n",name,&rest);
  if (rest < 0) { 
    fprintf(stderr,"no name cfg=%s\n",config);
    return 0;  
  } else {
    config += rest;
  }

  snprintf(lib,sizeof(lib),"%s/lib%s_%s.so",get_lib_dir(),base,name);
  snprintf(sym,sizeof(sym),"%s_%s",base,name);

  void *dl = dlopen(lib, RTLD_LAZY);
  if (dl == 0) {
    fprintf(stderr,"no shared library %s\n",lib);
    return 0;
  }

  builder_t *builder = (builder_t*) dlsym(dl,sym);
  if (builder == 0) {
    fprintf(stderr,"missing builder %s in library %s\n",sym,lib);
    return 0;
  }

  return builder(config);
}
