#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "reader.h"

char *pat1 = "this is a test.";
char *pat2= "iiae.";

reader_t* rng_skip(const char *config);

void test_skip0() 
{
  FILE *out = fopen("tmp/skip.dat","w");
  fwrite(pat1,strlen(pat1),1,out);
  fclose(out);

  reader_t *rng=rng_skip("skip=16 keep=8 rng=(reader tmp/skip.dat)");
  uint8_t tmp[4096];
  assert(reader_read(rng,tmp,sizeof(tmp)) == strlen(pat2));
  assert(strncmp((char*)tmp,pat2,strlen(pat2)) == 0);
  reader_close(rng);
}

void test_skip1() 
{
  int i,n = 100000;
  FILE *out = fopen("tmp/skip.dat","w");
  for (i=0; i<n; ++i) {
    fwrite(pat1,strlen(pat1),1,out);
  }
  fclose(out);

  reader_t *rng=rng_skip("skip=16 keep=8 rng=(reader tmp/skip.dat)");
  uint8_t tmp[4096];
  for (i=0; i<n; ++i) {
    assert(reader_read(rng,tmp,strlen(pat2)) == strlen(pat2));
    assert(strncmp((char*)tmp,pat2,strlen(pat2)) == 0);
  }
  reader_close(rng);
}


int main()
{
  test_skip0();
  test_skip1();
  printf("ok\n");
  return 0;
}
