#include "path_to_self.h"

#include <string.h>

#ifdef __APPLE__

#include <mach-o/dyld.h>
#include <string.h>
#include <stdlib.h>

char *path_to_self()
{
  char tmp[128];
  uint32_t size=2;
  if (_NSGetExecutablePath(tmp,&size) == 0) {
    return strdup(tmp);
  } else {
    char *ans = (char*) malloc(size);
    _NSGetExecutablePath(ans,&size);
    return ans;
  }
}

#endif

#ifdef __linux__

#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>

char *path_to_self()
{
  char tmp[PATH_MAX];
  FILE *in = fopen("/proc/self/exe","rb");
  ssize_t size = fread(tmp,sizeof(tmp)-1,1,in);
  if (size >= 0) {
    tmp[size-1]=0;
  } else {
    tmp[0]=0;
  }
  fclose(in);
  return strdup(tmp);
}

#endif

