#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "rng_load.h"
#include "stats_load.h"

typedef struct {
  stats_t *stats;
  reader_t *rng;
  unsigned nclean;
  void *clean[4096];
} test_t;

test_t * setup(const char *rng_config, const char *stats_config) {
  test_t * test = (test_t*) malloc(sizeof(test_t));
  assert(test != 0);
  memset(test,0,sizeof(test_t));
  test->rng = rng_load(rng_config);
  if (test->rng == 0) {
    fprintf(stderr,"could not load rng: %s\n",rng_config);
    exit(1);
  }
  test->stats = stats_load(stats_config);
  if (test->stats == 0) {
    fprintf(stderr,"coult not load stats: %s\n",stats_config);
    exit(1);
  }
  return test;
}

void run(test_t *test) {
  stats_run(test->stats,test->rng);
}

double get_double(test_t *test, const char *name) {
  return stats_get_double_by_name(test->stats,name);
}

void set_double(test_t *test, const char *name, double value) {
  assert (stats_set_double_by_name(test->stats,name,value) == 1); 
}

char *get_string(test_t *test, const char *name) {
  char *value = stats_get_string_by_name(test->stats,name);
  if (value != 0) {
    test->clean[test->nclean++]=value;
    assert(test->nclean++ <=sizeof(test->clean));
  }
  return value;
}

void cleanup(test_t *test) {
  reader_close(test->rng);
  stats_close(test->stats);
  unsigned i;
  for (i=0; i<test->nclean; ++i) {
    free(test->clean[i]);
  }
  free(test);
}


int get_df(test_t *test) {
  double df = get_double(test,"df");
  assert (floor(df) == df);
  assert (0 <= df);
  return (int) df;
}

int get_samples(test_t *test) {
  double samples = get_double(test,"samples");
  assert (isnan(samples) || floor(samples) == samples);
  assert (isnan(samples) || samples >= 0);
  return samples;
}

int get_sample(test_t *test) {
  double sample = get_double(test,"sample");
  double samples = get_samples(test);
  assert (floor(sample) == sample);
  assert (0 <= sample && (isnan(samples) || sample <= samples));
  return (int) sample;
}

double get_limit(test_t *test) {
  double limit = get_double(test,"limit");
  assert (isnan(limit) || limit > 0);
  return limit;
}

double get_progress(test_t *test) {
  double progress = get_double(test,"progress");
  assert (isnan(progress) || progress > 0);
  return progress;
}

double get_zluck(test_t *test) {
  return get_double(test,"zluck");
}

double get_luck(test_t *test) {
  double zluck=get_zluck(test);
  double luck=get_double(test,"luck");
  double nluck=0.5*(1+erf(zluck));
  assert (fabs(luck-nluck) < 1e-10);
  return luck;
}

double get_nluck(test_t *test) {
  double zluck=get_zluck(test);
  double nluck=get_double(test,"nluck");
  double luck=0.5*(1+erf(zluck));
  assert (fabs(luck-nluck) < 1e-10);
  return nluck;
}

void test_setup()
{
  const char *rng="reader /dev/urandom";
  const char *stats = "repeat samples=32 limit=6 stats=(max64 samples=10 use0=5 skip0=3 use1=3 skip1=5 offset=129)";

  test_t *test = setup(rng,stats);

  assert(strcmp(get_string(test,"name"),"repeat")==0);
  assert(get_double(test,"samples") == 32);
  assert(get_double(test,"limit") == 6);
  assert(get_double(test,"df") == 0);
  assert(isnan(get_double(test,"zluck")));
  assert(isnan(get_double(test,"luck")));
  assert(isnan(get_double(test,"nluck")));

  run(test);

  assert(get_samples(test) == 32);
  assert(get_sample(test) == 32);
  assert(get_df(test) == 32);
  assert(fabs(get_zluck(test)) < get_limit(test));

  printf("config: %s\n",get_string(test,"config"));
  printf("results: %s\n",get_string(test,"results"));

  cleanup(test);
}

void test_limit() {
  const char *rng="reader /dev/zero";
  const char *stats = "repeat samples=32 limit=6 stats=(max64 samples=10 use0=5 skip0=3 use1=3 skip1=5 offset=129)";

  test_t *test = setup(rng,stats);

  run(test);

  assert(get_samples(test) == 32);
  assert(get_sample(test) < 32);
  assert(get_df(test) < 32);
  assert(fabs(get_zluck(test)) >= get_limit(test));

  printf("config: %s\n",get_string(test,"config"));
  printf("results: %s\n",get_string(test,"results"));

  cleanup(test);
}

int main(int argc, char *argv[])
{
  test_setup();
  test_limit();
  printf("ok\n");
}
