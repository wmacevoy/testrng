#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "reader.h"
#include "stats.h"
#include "stats_max.h"
#include "rng_rdrand.h"


static inline int strcmpfree(char *p, const char *lit) {
  int ans = strcmp(p,lit);
  free(p);
  return ans;
}

void test_stats_max0()
{
  stats_t *s = stats_max("samples=10 use0=5 skip0=3 use1=3 skip1=5 offset=ab");
  printf("samples=%1.0lf\n",stats_get_double(s,"samples"));
  assert(stats_get_double(s,"samples") == 10);
  assert(stats_get_double(s,"use0") == 5);
  assert(stats_get_double(s,"use1") == 3);
  assert(stats_get_double(s,"skip0") == 3);
  assert(stats_get_double(s,"skip1") == 5);
  assert(strcmpfree(stats_get_string(s,"offset"),"ab")==0);

  reader_t *rng=rng_rdrand();

  stats_run(s,rng);

  double gap = stats_get_double(s,"gap");
  char *max = stats_get_string(s,"max");
  printf("gap=%lf max=%s\n",gap,max);
  free(max);

  stats_close(s);
  reader_close(rng);
}

void test_stats_max1()
{
  int bins[256];
  {int i; for (i=0; i<256; ++i) { bins[i]=0; } }
  stats_t *s = stats_max("samples=10 use0=5 skip0=3 use1=3 skip1=5 offset=00");
  reader_t *rng=rng_rdrand();

  int n=10;
  {int i; for (i=0; i<n; ++i) {
      stats_run(s,rng);
      char *maxstr = stats_get_string(s,"max");
      int max=strtol(maxstr,0,16);
      printf("test %d max=%s (%d)\n",i,maxstr,max);
      ++bins[max];
      free(maxstr);
    }}

  {int i; for (i=0; i<256; ++i) {
      printf("i=%d, count=%d, freq=%lf\n",i,bins[i],((double)bins[i])/((double)n));
    }}

  stats_close(s);
  reader_close(rng);
}

int main(int argc, char *argv[])
{
  test_stats_max0();
  test_stats_max1();
  printf("ok\n");
}
