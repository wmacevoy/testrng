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
  ssize_t status = readlink("/proc/self/exe", tmp, PATH_MAX);
  if (status >= 0) {
    tmp[status]=0;
    return strdup(tmp);
  } else {
    return 0;
  }
}

#endif

#ifdef _WIN32

#include <Windows.h>

char *path_to_self() {
  DWORD tcapacity = 0;
  TCHAR *tchars = 0;
  DWORD tsize = 0;
  do {
    tcapacity += 4096;
    TCHAR* _tchars = (TCHAR*) realloc(tchars,sizeof(TCHAR)*tcapacity);
    if (_tchars == 0) { free(tchars); return 0; }
    tsize = GetModuleFileName(0,&tchars,tcapacity-1);
  } while (tsize >= tcapacity-1);
  
  tchars[tsize]=0;

  if (sizeof(TCHAR) != 1) {
    int csize = WideCharToMultiByte(CP_UTF8,WC_COMPOSITECHECK|WC_DEFAULTCHAR,tchars,tsize+1,NULL,0,NULL,NULL);
    char *chars = (char*) malloc(csize);
    if (chars != 0) {
      WideCharToMultiByte(CP_UTF8,WC_COMPOSITECHECK|WC_DEFAULTCHAR,tchars,tsize+1,chars,csize,NULL,NULL);
    }
    free(tchars);
    return chars;
  } else {
    char *chars = strdup(tchars);
    free(tchars);
    return chars;
  }
}

#endif

