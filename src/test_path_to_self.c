#include "path_to_self.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

char *magic = "TEST_PATH_TO_SELF";

void test_self() {
  char tmp[4096];
  char *p=path_to_self();
  snprintf(tmp,sizeof(tmp),"strings \"%s\" | grep \"%s\" >/dev/null",p,magic);
  free(p);
  assert (system(tmp) == 0);
}

int main() {
  test_self();
  test_self();
  printf("ok\n");
  return 0;
}
