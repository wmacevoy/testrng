#include "path_to_self.h"


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

#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

char *path_to_self()
{
  char tmp[PATH_MAX];
  if (readlink("/proc/self/exe", tmp, PATH_MAX) >= 0) {
    return strdup(tmp);
  } else {
    return 0;
  }
}

#endif

