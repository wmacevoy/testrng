#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "reader.h"
#include "rng_load.h"

char *pat1 = "0123abcdABCD";
char *pat2= "2bc01ABCa";
char *pat3 = "2bc01ABCa2bc01ABCa2bc01ABCa2bc01ABCa2bc01ABCa2bc01ABCa2bc01ABCa2bc01ABCa2bc01ABCa2bc01ABCa";

void test_mix0() 
{
  FILE *out = fopen("tmp/mix32.dat","w");
  fwrite(pat1,strlen(pat1),1,out);
  fclose(out);

  reader_t *rng=rng_load("mix32 mix=24 rng=(reader tmp/mix32.dat)");
  uint8_t tmp[4096];
  ssize_t got=reader_read(rng,tmp,sizeof(tmp));
  tmp[got]=0;
  assert(got == strlen(pat2));
  assert(strncmp((char*)tmp,pat2,strlen(pat2)) == 0);
  reader_close(rng);
}

void test_mix1()
{
  int n=10;
  {
    int i;
    FILE *out = fopen("tmp/mix32.dat","w");
    for (i=0; i<n; ++i) {
      fwrite(pat1,strlen(pat1),1,out);
    }
    fclose(out);
  }

  char tmp[1024];
  reader_t *rng=rng_load("mix32 mix=24 rng=(reader tmp/mix32.dat)");
  ssize_t got=reader_read(rng,tmp,sizeof(tmp));
  assert(got == strlen(pat3));
  tmp[got]=0;
  assert(strcmp(tmp,pat3)==0);

  reader_close(rng);
}

int main()
{
  test_mix0();
  test_mix1();
  printf("ok\n");
  return 0;
}
