#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "reader.h"
#include "stats.h"
#include "stats_max64.h"
#include "rng_rdrand.h"


static inline int strcmpfree(char *p, const char *lit) {
  int ans = strcmp(p,lit);
  free(p);
  return ans;
}

void test_stats_max64_0()
{
  stats_t *s = stats_max64("samples=10 use0=5 skip0=3 use1=3 skip1=5 offset=ab");
  printf("samples=%1.0lf\n",stats_get_double(s,stats_get_double_index(s,"samples")));
  assert(stats_get_double(s,stats_get_double_index(s,"samples")) == 10);
  assert(stats_get_double(s,stats_get_double_index(s,"use0")) == 5);
  assert(stats_get_double(s,stats_get_double_index(s,"use1")) == 3);
  assert(stats_get_double(s,stats_get_double_index(s,"skip0")) == 3);
  assert(stats_get_double(s,stats_get_double_index(s,"skip1")) == 5);
  assert(stats_get_double(s,stats_get_double_index(s,"offset")) == 0xab);

  printf("offset=%s\n",stats_get_string(s,stats_get_string_index(s,"offset")));

  assert(strcmpfree(stats_get_string(s,stats_get_string_index(s,"offset")),"ab")==0);
  reader_t *rng=rng_rdrand();

  stats_run(s,rng);

  double gap = stats_get_double(s,stats_get_double_index(s,"gap"));
  char *max = stats_get_string(s,stats_get_double_index(s,"max"));
  printf("gap=%lf max=%s\n",gap,max);
  free(max);

  stats_close(s);
  reader_close(rng);
}

int by_prob(const void *v, const void *w) {
  return copysign(1,((double*)w)[1]-((double*)v)[1]);
}

void test_stats_max64_1()
{
  stats_t *s = stats_max64("samples=3 use0=3 skip0=3 use1=5 skip1=5 offset=0");
  reader_t *rng=rng_rdrand();

  int bits = (int) (stats_get_double(s,stats_get_double_index(s,"use0")) + stats_get_double(s,stats_get_double_index(s,"use1")));
  printf("bits=%d\n",bits);
  int bins[1<<bits];
  {int i; for (i=0; i<(1<<bits); ++i) { bins[i]=0; } }

  double zl = 0;
  double df = 0;
  int n=100000;
  {int i; for (i=0; i<n; ++i) {
      stats_run(s,rng);
      int max=(int) stats_get_double(s,stats_get_double_index(s,"max"));
      if (df == 0) {
        zl = stats_get_double(s,stats_get_double_index(s,"zluck"));
        df = stats_get_double(s,stats_get_double_index(s,"df"));
      } else {
        double zl1 = stats_get_double(s,stats_get_double_index(s,"zluck"));
        double df1 = stats_get_double(s,stats_get_double_index(s,"df"));
        zl = sqrt(pow(zl+sqrt(df-0.5),2)+pow(zl1+sqrt(df1-0.5),2))-sqrt(df+df1-0.5);
        df = df + df1;
      }
      ++bins[max];
    }}

  printf("zluck=%lf, df=%lf, normal luck=%lf\n",zl,df,0.5*(1+erf(zl)));

  {int i; for (i=0; i<(1<<bits); ++i) {
      if (i > 0) printf(",");
      printf("%lf",((double)bins[i])/((double)n));
    }}
  printf("\n");

  double N = 1<<bits;
  double S = stats_get_double(s,stats_get_double_index(s,"samples"));

  double *tab = (double*) malloc(5*N*sizeof(double));
  double sum1=0,sum2=0;
  {int i; for (i=0; i<(1<<bits); ++i) {
      double M = i+1;
      double k = N-M;
      double p = pow(M/N,S)*(1-pow((M-1)/M,S));
      //      (1-k/N)^S * (1-(1-1/M)^S)
      //      (1-k/N)^(N/k*S*k/N)*(1-(1-1/M)^M*S/M)
      //
      //      exp(-S*k/N)*(1-exp(-S/M))
      //
      double q = exp(-S*k/N)*(1-exp(-S/(N-k)));
      sum1 += p*(i);
      sum2 += p*(i*i);
      tab[5*i+0]=i;
      tab[5*i+1]=p;
      tab[5*i+2]=0; // |Omega|
      tab[5*i+3]=0; // |omega|
      tab[5*i+4]=q;
    }}

  double mu = sum1;
  double Sigma = sum2-sum1*sum1;

  printf("mu=%lf, Sigma=%lf\n",mu,Sigma);

  qsort(tab,N,5*sizeof(double),by_prob);

  {int j=0; while (j<N) {
      {
        int k=1; 
        while ((j+k)<N 
               && (tab[5*(j+k)+1] > tab[5*j+1]*(1-1e-5))) { 
          ++k; 
        }
        double Omega = (j == 0) ? 0 : (tab[5*(j-1)+2]+tab[5*(j-1)+3]);
        double omega = k*tab[5*j+1];
        int i=j;
        j += k;
        while (i<j) { 
          tab[5*i+2] = Omega; 
          tab[5*i+3] = omega; 
          //          tab[5*i+4] = q;
          ++i;
        }
      }
    }
  }
  {int i; for (i=0; i<N; ++i) {
      double M = tab[5*i+1]+1;
      double K = N-M;
      double S = stats_get_double(s,stats_get_double_index(s,"samples"));

      double luck = 1-exp(-S/N*K);
    printf("i=%d,p=%lf,|Omega|=%lf,|omega|=%lf,L=%lf vs %lf\n",(int) tab[5*i+0], 
           tab[5*i+1], tab[5*i+2], tab[5*i+3], tab[5*i+4],luck);
    }}

  stats_close(s);
  reader_close(rng);
}

int main(int argc, char *argv[])
{
  test_stats_max64_0();
  test_stats_max64_1();
  printf("ok\n");
}
