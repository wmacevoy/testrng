#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include "bits.h"

#include "stats_max64.h"

typedef struct stats_max64 {
  stats_t base;
  int samples;

  uint64_t max;
  uint64_t offset;
  uint16_t use0;
  uint16_t skip0;
  uint16_t use1;
  uint16_t skip1;
  double mu;
  double sigma;
  double value;
  uint8_t changed;
} stats_max64_t;

#define ME ((stats_max64_t*)(me))

static inline int get_bytes(stats_t *me) {
  int m = (ME->use0 + ME->use1);
  int n = (m + 7)/8;
  return n;
}

static inline int get_bits(stats_t *me) {
  return (ME->use0 + ME->use1);
}

static char *string(stats_t *me, uint64_t r) {
  int n = get_bytes(me);
  char *ans = (char*) malloc(2*n+1);
  {int i; for (i=0; i<n; ++i) { sprintf(ans+2*i,"%02x",((uint8_t*)&r)[i]); } }
  return ans;
}

static void store(stats_t *me, uint64_t *r, const char *value) {
  *r = strtol(value,0,16);
}


enum DoubleIndexes {
  DISamples,
  DIUse0,
  DIUse1,
  DISkip0,
  DISkip1,
  DIOffset,
  DIMax,
  DIValue,
  DILuck,
  DIZLuck,
  DINLuck,
  DIDF,
};

enum StringIndexes {
  SIName = 0x10000,
  SIConfig,
  SIResults,
  SIEnd
};


static void stats_max64_config(stats_t *me, const char *args) {
  char name[4096];
  char value[4096];

  for (;;) {
    int delta = -1;
    sscanf(args," %[^\t =] = %s %n",name,value,&delta);
    if (delta <= 0) break;
    args += delta;

    int si = me->get_string_index(me,name);
    if (si >= 0) {
      me->set_string(me,si,value);
      continue;
    }

    int di = me->get_double_index(me,name);
    if (di >= 0) {
      me->set_double(me,di,atof(value));
      continue;
    }
  }
}

static double value(stats_t *me) {
  int bits = get_bits(me);
  return (~((ME->max+1) << (64-bits))+1)/pow(2,64);
}

static double luck(stats_t *me) {
  int bits = get_bits(me);
  double N = pow(2,bits);
  int S = ME->samples;
  double M = ME->max + 1;
  double k = N-M;

  if (k <= 1e6) {
      double ans = 0.5*pow(M/N,S)*(1-pow((M-1)/M,S));
      while (--k >= 0) {
        ans += pow((N-k)/N,S)*(1-pow((N-k-1)/(N-k),S));
      }
      return ans;
  } else {
    return 1-exp(-(S/N)*(k+0.5));
  }
}

static void moments(stats_t *me) {
  if (!ME->changed) return;

  int bits = get_bits(me);

  double N = pow(2,bits);
  double S = ME->samples;

  double Z = (1+1/(4*N))/(2*N);
  double G = 1/(S+1)*exp(-(S+1)*Z);
  double H = 1/(S+2)*exp(-(S+2)*Z);
  
  double muEst = G;
  double sigmaEst = sqrt((2-G-1/N)*G-2*H);

  if (bits <= 24) {
    int i;
    double sum1=0;
    double sum2=0;
    for (i=N-1; i>=0; --i) {
      double p = pow((N-i)/N,S)*(1-pow((N-i-1)/(N-i),S));
      double g = ((double) i)/((double) N);
      sum1 += p*g;
      sum2 += p*g*g;
    }

    double mu=sum1;
    double sigma=sqrt(sum2-sum1*sum1);

    ME->mu=mu;
    ME->sigma=sigma;
    ME->changed=0;
  } else {
    ME->mu = muEst;
    ME->sigma = sigmaEst;

    ME->changed = 0;
  }
}

static double df(stats_t *me) {
  return 1;
}

static double zluck(stats_t *me) {
  if (ME->value != NAN) {
    moments(me);
    double x = value(me);
    return (x-ME->mu)/ME->sigma-sqrt(df(me)-0.5);
  } else {
    return NAN;
  }
}

static double nluck(stats_t *me) {
  if (ME->value != NAN) {
    return 0.5*(1+erf(zluck(me)));
  } else {
    return NAN;
  }
}

static int stats_max64_get_double_index(stats_t *me, const char *name) {
  if (strcmp(name,"samples")==0) { return DISamples; }
  if (strcmp(name,"use0")==0) { return DIUse0; }
  if (strcmp(name,"use1")==0) { return DIUse1; }
  if (strcmp(name,"skip0")==0) { return DISkip0; }
  if (strcmp(name,"skip1")==0) { return DISkip1; }
  if (strcmp(name,"offset")==0) { return DIOffset; }
  if (strcmp(name,"value")==0) { return DIValue; }
  if (strcmp(name,"max")==0) { return DIMax; }
  if (strcmp(name,"luck")==0) { return DILuck; }
  if (strcmp(name,"zluck")==0) { return DIZLuck; }
  if (strcmp(name,"nluck")==0) { return DINLuck; }
  if (strcmp(name,"df")==0) { return DIDF; }
  return -1;
}

static int stats_max64_get_string_index(stats_t *me, const char *name) {
  if (strcmp(name,"name")==0) { return SIName; }
  if (strcmp(name,"config")==0) { return SIConfig; }
  if (strcmp(name,"results")==0) { return SIResults; }

  return -1;
}

static double stats_max64_get_double(stats_t *me, int index) {
  switch(index) {
  case DIUse0: return ME->use0;
  case DIUse1: return ME->use1;
  case DISkip0: return ME->skip0;
  case DISkip1: return ME->skip1;
  case DIOffset: return ME->offset;
  case DISamples: return ME->samples;
  case DIValue: return ME->value;
  case DILuck: return luck(me);
  case DIZLuck: return zluck(me);
  case DINLuck: return nluck(me);
  case DIDF: return df(me);
  case DIMax: return ME->max;
  }
  return NAN;
}

static int stats_max64_set_double(stats_t *me, int index, double value) {
  ME->changed=1;
  switch(index) {
  case DIUse0: ME->use0=value; return 1;
  case DIUse1: ME->use1=value; return 1;
  case DISkip0: ME->skip0=value; return 1;
  case DISkip1: ME->skip1=value; return 1;
  case DIOffset: ME->offset=value; return 1;
  case DISamples: ME->samples=value; return 1;
  }
  return 0;
}

static int stats_max64_set_string(stats_t *me, int index, const char *value) {
  return 0;
}

static char *name(stats_t* me) { return strdup("max64"); }

static char *config(stats_t *me) {
  char tmp[4096];
  int bits = ME->use0 + ME->use1;

  moments(me);

  snprintf(tmp,sizeof(tmp),
           "samples=%d"
           " use0=%d skip0=%d use1=%d skip1=%d"
           " offset=%" PRIX64,
           ME->samples,
           ME->use0,ME->skip0,ME->use1,ME->skip1,
           ME->offset);

  return strdup(tmp);
}

static char *results(stats_t *me) {
  char tmp[4096];
  int bits = get_bits(me);

  moments(me);

  snprintf(tmp,sizeof(tmp),
           "mu=%lg sigma=%lg"
           " value=%lg zluck=%lg nluck=%lg df=%lf luck=%lg",
           ME->mu,ME->sigma,
           me->get_double(me,DIValue),
           me->get_double(me,DIZLuck),
           me->get_double(me,DINLuck),
           me->get_double(me,DIDF),
           me->get_double(me,DILuck));

  return strdup(tmp);
}

static char *stats_max64_get_string(stats_t *me, int index) {
  switch(index) {
  case SIName: return strdup("max64");
  case SIConfig: return config(me);
  case SIResults: return results(me);
  }
  return 0;
}

static void stats_max64_run(stats_t *me, reader_t *src) {
  bits_t *b = bits(src,BITS_NO_CLOSE);
  int mbits = get_bits(me);
  uint64_t mask = ~((~((uint64_t)(0))) << mbits);
  int n = get_bytes(me);
  uint64_t x;
  uint64_t max = 0;

  {int i; for (i=0; i<ME->samples; ++i) {
      x=0;
      bits_read(b,&x,0,ME->use0);
      bits_skip(b,ME->skip0);
      bits_read(b,&x,ME->use0,ME->use1);
      bits_skip(b,ME->skip1);
      x = (x + ME->offset) & mask;
      if (x > max) {
        max=x;
      }
    }}
  ME->max = max;
  ME->value = value(me);
  bits_close(b);
}

static void stats_max64_close(stats_t *me) {
  free(ME);
}

stats_t* stats_max64(const char *cfg)
{
  stats_t *me =
    (stats_t*)malloc(sizeof(stats_max64_t));
  
  if (me == 0) return (stats_t*)0;

  ME->use0 = 12;
  ME->skip0 = 4;
  ME->use1 =  7;
  ME->skip1 = 12;
  ME->max = 0;
  ME->offset = 0;
  ME->samples = 16;
  ME->changed=1;
  ME->mu=0;
  ME->sigma=0;
  ME->value=NAN;

  ME->base.config = stats_max64_config;
  ME->base.run = stats_max64_run;
  ME->base.get_double_index = stats_max64_get_double_index;
  ME->base.get_string_index = stats_max64_get_string_index;
  ME->base.get_double = stats_max64_get_double;
  ME->base.get_string = stats_max64_get_string;
  ME->base.set_double = stats_max64_set_double;
  ME->base.set_string = stats_max64_set_string;
  ME->base.close = stats_max64_close;

  if (cfg != 0) {
    stats_config(me,cfg);
  }

  return me;
}

