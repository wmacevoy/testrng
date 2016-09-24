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
  ssize_t got=reader_read(rng,tmp,sizeof(tmp));
  printf("got %d bytes\n",(int) got);
  tmp[got]=0;
  printf("got '%s'\n",tmp);
  assert(got == strlen(pat2));
  assert(strncmp((char*)tmp,pat2,strlen(pat2)) == 0);
  reader_close(rng);
}

void test_skip1() 
{
  int n=1000000;
  {
    int i;
    FILE *out = fopen("tmp/skip.dat","w");
    for (i=0; i<n; ++i) {
      fwrite(pat1,strlen(pat1),1,out);
    }
    fclose(out);
  }

  int nn = n*strlen(pat2);
  printf("nn=%d\n",nn);

  int chunk;
  for (chunk=1; chunk<=10000; chunk *= 2) {
    int i;
    reader_t *rng=rng_skip("skip=16 keep=8 rng=(reader tmp/skip.dat)");
    uint8_t tmp[chunk+1];

    for (i=0; i<nn-chunk; i += chunk) {
      int j;
      int got =reader_read(rng,tmp,chunk);
      if (got != chunk) {
        printf("i=%d nn=%d chunk=%d\n",(int) i,(int)nn,(int) chunk);
        printf("got %d and wanted %d at i=%d\n",(int)got,(int)chunk,i);
        assert(0);
      }
      for (j=0; j<chunk; ++j) {
        int ii=(i+j)%strlen(pat2);
        if (tmp[j]!=pat2[ii]) {
          tmp[got]=0;
          printf("got(%s) %c and wanted %c at i=%d\n",tmp,(char)tmp[j],(char)pat2[ii],i);
          assert(0);
        }
        assert(tmp[j]==pat2[(i+j)%strlen(pat2)]);
      }
    }
    reader_close(rng);
  }
}

int main()
{
  test_skip0();
  test_skip1();
  printf("ok\n");
  return 0;
}
