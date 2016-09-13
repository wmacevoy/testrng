#include "path_to_self.h"

#include <stdlib.h>

int main() {
  char *p=path_to_self();
  printf("path: %s\n", p);
  free(p);
  return 0;
}
