#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "reader.h"
#include "stats.h"
#include "stats_max64.h"
#include "rng_rdrand.h"

typedef struct {
  stats_t *stats;
  reader_t *rng;
  unsigned nclean;
  void *clean[4096];
} test_t;

test_t * setup(const char *config) {
  test_t * test = (test_t*) malloc(sizeof(test_t));
  assert(test != 0);
  memset(test,0,sizeof(test_t));
  test->rng = rng_rdrand();
  test->stats = stats_max64(config);
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
  for (unsigned i=0; i<test->nclean; ++i) {
    free(test->clean[i]);
  }
  free(test);
}

int get_use0(test_t *test) {
  double use0 = get_double(test,"use0");
  assert (floor(use0) == use0 && 0 <= use0 && use0 <= 64);
  return (int) use0;
}

int get_use1(test_t *test) {
  double use1 = get_double(test,"use1");
  assert (floor(use1) == use1 && 0 <= use1 && use1 <= 64);
  return (int) use1;
}

int get_bits(test_t *test) {
  double bits = get_use0(test) + get_use1(test);
  assert (0 <= bits && bits <= 64);
  return (int) bits;
}

double get_N(test_t *test) {
  return pow(2,get_bits(test));
}

double get_M(test_t *test) {
  double Mm1 = get_double(test,"max");
  if (!isnan(Mm1)) {
    assert (floor(Mm1) == Mm1);
    assert (0 <= Mm1 && Mm1 < get_N(test));
    return Mm1 + 1.0;
  } else {
    return NAN;
  }
}

double get_S(test_t *test) {
  double samples = get_double(test,"samples");
  assert (floor(samples) == samples);
  return samples;
}

double get_value(test_t *test) {
  double value = get_double(test,"value");
  if (!isnan(value)) {
    printf("value=%lg\n",value);
    assert(0 <= value && value <= 1.0);
  }
  return value;
}

double get_prob(test_t *test, double M) {
  double N=get_N(test);
  double S=get_S(test);
  double P1=pow(M/N,S)*(1-pow((M-1)/M,S));
  double x = (N-M)/N;
  double _x = get_double(test,"value");
  double _M = get_M(test);
  set_double(test,"value",x);
  assert(M == get_M(test));
  double P2=exp(get_double(test,"lnp"));
  assert(fabs(P1-P2)<1e-10);
  set_double(test,"value",_x);
  assert((isnan(_M) && isnan(get_M(test))) || _M == get_M(test));  

  return P2;
}

int get_df(test_t *test) {
  int df = get_double(test,"df");
  assert (floor(df) == df);
  assert (1 <= df);
  return (int) df;
}

double get_zluck(test_t *test) {
  return get_double(test,"zluck");
}

double get_mean(test_t *test) {
  double N=get_N(test);
  double S=get_S(test);

  int i;
  double sum1 = 0.0;

  for (i=((int) N)-1; i>=0; --i) {
    double M = N-i;
    double g = (N-M)/N;
    double p = pow(M/N,S)*(1-pow((M-1)/M,S));
    sum1 += p*g;
  }

  double mean1 = sum1;
  double mean2 = get_double(test,"mean");

  assert(fabs(mean1-mean2) < 1e-10);

  return mean2;
}

double get_variance(test_t *test) {
  double N=get_N(test);
  double S=get_S(test);

  int i;
  double sum1 = 0.0;
  double sum2 = 0.0;

  for (i=((int)N)-1; i>=0; --i) {
    double M = N-i;
    double g = (N-M)/N;
    double p = pow(M/N,S)*(1-pow((M-1)/M,S));

    sum1 += p*g;
    sum2 += p*g*g;
  }

  double variance1 = sum2-sum1*sum1;
  double variance2 = get_double(test,"variance");

  assert(fabs(variance1-variance2) < 1e-10);

  return variance2;
}


void test_setup()
{
  test_t *test = setup("samples=10 use0=5 skip0=3 use1=3 skip1=5 offset=129");

  assert(strcmp(get_string(test,"name"),"max64")==0);
  assert(get_double(test,"samples") == 10);
  assert(get_double(test,"use0") == 5);
  assert(get_double(test,"use1") == 3);
  assert(get_double(test,"skip0") == 3);
  assert(get_double(test,"skip1") == 5);
  assert(get_double(test,"offset") == 129);

  assert (get_bits(test) == 8);
  assert (get_N(test) == 256);
  assert (get_S(test) == 10);

  double mean0=get_mean(test);
  double variance0=get_variance(test);

  assert (isnan(get_M(test)));
  assert (isnan(get_value(test)));

  run(test);

  assert (0 <= get_value(test) && get_value(test) <= 1.0);

  assert (get_mean(test)==mean0);
  assert (get_variance(test)==variance0);

  assert (get_M(test) >= 1 && get_M(test) <= get_N(test));

  printf("config: %s\n",get_string(test,"config"));
  printf("results: %s\n",get_string(test,"results"));

  cleanup(test);
}

void test_probs()
{
  test_t *test = setup("samples=3 use0=3 skip0=3 use1=5 skip1=5 offset=63");
  int trials = 100000;

  int trial;
  for (trial = 0; trial < trials; ++trial) {
    run(test);
    get_prob(test,get_M(test));
  }

  cleanup(test);
}

void test_bins()
{
  test_t *test = setup("samples=3 use0=3 skip0=3 use1=5 skip1=5 offset=63");

  int N=(int) get_N(test);
  int trials=10000;
  int trial;

  int bins[N];
  memset(bins,0,sizeof(bins));

  for (trial=0; trial<trials; ++trial) {
    run(test);
    ++bins[(int)(get_M(test)-1)];
  }

  int i;
  double abserr = 10/sqrt(trials);
  double maxerr = 0;

  for (i=0; i<N; ++i) {
    double M=i+1;
    double p = get_prob(test,M);
    double q = ((double) bins[i])/((double) trials);
    double err = fabs(p-q);
    if (err > maxerr) maxerr=err;
  }

  printf("maxerr=%lg\n",maxerr);

  assert (maxerr < 10/sqrt(trials));
  cleanup(test);
}

int by_prob(const void *v, const void *w) {
  return copysign(1,((double*)w)[1]-((double*)v)[1]);
}

void test_nluck()
{
  test_t *test = setup("samples=3 use0=3 skip0=3 use1=5 skip1=5 offset=63");

  int trials=10000;
  int trial;

  double total_zl = 0;
  int total_df = 0;

  for (trial=0; trial<trials; ++trial) {
      run(test);
      double zl = get_zluck(test);
      double df = get_df(test);
      if (total_df == 0) {
        total_df = df;
        total_zl = zl;
      } else {
        total_zl = sqrt(pow(total_zl+sqrt(total_df-0.5),2)+pow(zl+sqrt(df-0.5),2))-sqrt(total_df+df-0.5);
        total_df = total_df + df;
      }
  }

  printf("totals: zluck=%lf, df=%d, normal luck=%lf\n",total_zl,total_df,0.5*(1+erf(total_zl)));

  cleanup(test);
}

void test_luck()  {
  test_t *test = setup("samples=3 use0=3 skip0=3 use1=5 skip1=5 offset=63");

  double N = get_N(test);
  double S = get_S(test);
  int cols = 5;

  double *tab = (double*) malloc(cols*N*sizeof(double));

  {
    int i;
    for (i=N-1; i>=0; --i) {
      double M = i+1;
      double k = N-M;
      double q = exp(-S*k/N)*(1-exp(-S/(N-k)));
      tab[cols*i+0]=i;
      tab[cols*i+1]=get_prob(test,M);
      tab[cols*i+2]=0; // |Omega|
      tab[cols*i+3]=0; // |omega|
      tab[cols*i+4]=0; // Luck
    }
  }

  qsort(tab,N,5*sizeof(double),by_prob);

  {
    int j=0; 
    while (j<N) {
      int k=1; 
      while ((j+k)<N 
             && (tab[cols*(j+k)+1] > tab[cols*j+1]*(1-1e-5))) { 
        ++k; 
      }
      double Omega = (j == 0) ? 0 : (tab[cols*(j-1)+2]+tab[cols*(j-1)+3]);
      double omega = k*tab[cols*j+1];
      int i=j;
      j += k;
      while (i<j) { 
        tab[cols*i+2] = Omega; 
        tab[cols*i+3] = omega; 
        tab[cols*i+4] = Omega+0.5*omega;
        ++i;
      }
    }
  }

  {
    int i; 
    for (i=0; i<N; ++i) {
      double M = tab[cols*i+0]+1;
      double x = (N-M)/N;
      set_double(test,"value",x);
      assert (get_M(test) == M);
      double p=get_prob(test,M);
      double Omega = tab[cols*i+2];
      double omega = tab[cols*i+3];
      double L1 = tab[cols*i+4];
      double L2 = get_double(test,"luck");
      assert (fabs(L1-L2) < 1e-10);
    }
  }

  cleanup(test);
}

int main(int argc, char *argv[])
{
  test_setup();
  test_probs();
  test_bins();
  test_nluck();
  test_luck();
  printf("ok\n");
}
