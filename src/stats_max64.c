#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bits.h"

#define STATS_MAX64_EXPORT __declspec(dllexport)
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
  uint8_t changed;
} stats_max64_t;

static inline int size(stats_max64_t *me) {
  int m = (me->use0 + me->use1);
  int n = (m + 7)/8;
  return n;
}

static char *string(stats_max64_t *me, uint64_t r) {
  int n = size(me);
  char *ans = (char*) malloc(2*n+1);
  {int i; for (i=0; i<n; ++i) { sprintf(ans+2*i,"%02x",((uint8_t*)&r)[i]); } }
  return ans;
}

static void store(stats_max64_t *me, uint64_t *r, const char *value) {
  *r = strtol(value,0,16);
}

enum StringIndexes {
  SIName,
  SISummary,
  SIOffset,
  SIEnd
};

enum DoubleIndexes {
  DISamples = SIEnd,
  DIUse0,
  DIUse1,
  DISkip0,
  DISkip1,
  DIOffset,
  DIGap,
  DIMax,
  DILuck,
  DIZLuck,
  DIDF,
};

static void stats_max64_config(stats_t *me, const char *args) {
  stats_max64_t *ME = (stats_max64_t*)me;

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

static double gap(stats_max64_t *me) {
  return (pow(2,64)-me->max)/pow(2,64);
}

static double luck(stats_max64_t *me) {
  stats_max64_t *ME = (stats_max64_t*)me;
  int bits = me->use0 + me->use1;
  double N = pow(2,bits);
  int S = me->samples;
  double M = me->max + 1;
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

static void moments(stats_max64_t *me) {
  int bits = me->use0 + me->use1;
  double N = pow(2,bits);
  int S = me->samples;
  double M = me->max + 1;
  double k = N-M;

  if (N <= 1e6) {
    int i;
    double sum1=0;
    double sum2=0;
    for (i=0; i<N; ++i) {
      double p = pow((N-i)/N,S)*(1-pow((N-i-1)/(N-i),S));
      sum1 += p*(i);
      sum2 += p*(i*i);
    }
    double mu=sum1;
    double sigma=sqrt(sum2-sum1*sum1);
    me->mu=mu;
    me->sigma=sigma;
    me->changed=0;
  } else {
    assert(0);
  }
}


static double zluck(stats_max64_t *me) {
  if (me->changed) moments(me);
  int bits = me->use0 + me->use1;
  double N = pow(2,bits);
  int S = me->samples;
  double M = me->max + 1;
  double k = N-M;
  return (k-me->mu)/me->sigma-sqrt(0.5);
}

static int stats_max64_get_double_index(stats_t *me, const char *name) {
  if (strcmp(name,"samples")==0) { return DISamples; }
  if (strcmp(name,"use0")==0) { return DIUse0; }
  if (strcmp(name,"use1")==0) { return DIUse1; }
  if (strcmp(name,"skip0")==0) { return DISkip0; }
  if (strcmp(name,"skip1")==0) { return DISkip1; }
  if (strcmp(name,"offset")==0) { return DIOffset; }
  if (strcmp(name,"gap")==0) { return DIGap; }
  if (strcmp(name,"max")==0) { return DIMax; }
  if (strcmp(name,"luck")==0) { return DILuck; }
  if (strcmp(name,"zluck")==0) { return DIZLuck; }
  if (strcmp(name,"df")==0) { return DIDF; }
  return -1;
}

static int stats_max64_get_string_index(stats_t *me, const char *name) {
  if (strcmp(name,"name")==0) { return SIName; }
  if (strcmp(name,"summary")==0) { return SISummary; }
  if (strcmp(name,"offset")==0) { return SIOffset; }
  return -1;
}

static double stats_max64_get_double(stats_t *me, int index) {
  stats_max64_t *ME = (stats_max64_t*)me;
  switch(index) {
  case DIUse0: return ME->use0;
  case DIUse1: return ME->use1;
  case DISkip0: return ME->skip0;
  case DISkip1: return ME->skip1;
  case DIOffset: return ME->offset;
  case DISamples: return ME->samples;
  case DIGap: return gap(ME);
  case DILuck: return luck(ME);
  case DIZLuck: return zluck(ME);
  case DIDF: return 1;
  case DIMax: return ME->max;

  }
  return NAN;
}

static void stats_max64_set_double(stats_t *me, int index, double value) {
  stats_max64_t *ME = (stats_max64_t*)me;
  ME->changed=1;
  switch(index) {
  case DIUse0: ME->use0=value; return;
  case DIUse1: ME->use1=value; return;
  case DISkip0: ME->skip0=value; return;
  case DISkip1: ME->skip1=value; return;
  case DIOffset: ME->offset=value; return;
  case DISamples: ME->samples=value; return;
  }
}

static char *stats_max64_get_string(stats_t *me, int index) {
  stats_max64_t *ME = (stats_max64_t*)me;
  ME->changed=1;
  switch(index) {
  case SIName: return strdup("stats_max");
  case SISummary: return strdup("rtmm");
  case SIOffset: return string(ME,ME->offset);
  }
  return 0;
}

static void stats_max64_set_string(stats_t *me, int index, const char *value) {
  stats_max64_t *ME = (stats_max64_t*)me;
  switch(index) {
  case SIOffset: store(ME,&ME->offset,value); return;
  }
}

static void stats_max64_run(stats_t *me, reader_t *src) {
  stats_max64_t *ME = (stats_max64_t*)me;
  bits_t *b = bits(src,BITS_NO_CLOSE);
  int mbits = ME->use0 + ME->use1;
  uint64_t mask = ~((~((uint64_t)(0))) << mbits);
  int n = size(ME);
  uint64_t value;
  uint64_t max = 0;

  {int i; for (i=0; i<ME->samples; ++i) {
      value=0;
      bits_read(b,&value,0,ME->use0);
      bits_skip(b,ME->skip0);
      bits_read(b,&value,ME->use0,ME->use1);
      bits_skip(b,ME->skip1);
      value = (value + ME->offset) & mask;
      if (value > max) {
        max=value;
      }
    }}
  ME->max = max;
  bits_close(b);
}

static void stats_max64_close(stats_t *me) {
  stats_max64_t *ME = (stats_max64_t*)me;
  free(ME);
}

stats_t* stats_max64(const char *cfg)
{
  printf("stats_max64(%s)\n",cfg);
  stats_max64_t *me =
    (stats_max64_t*)malloc(sizeof(stats_max64_t));
  
  if (me == 0) return (stats_t*)0;

  me->use0 = 8;
  me->skip0 = 0;
  me->use1 = 0;
  me->skip1 = 0;
  me->max = 0;
  me->offset = 0;
  me->samples = 100;
  me->changed=1;
  me->mu=0;
  me->sigma=0;

  me->base.config = stats_max64_config;
  me->base.run = stats_max64_run;
  me->base.get_double_index = stats_max64_get_double_index;
  me->base.get_string_index = stats_max64_get_string_index;
  me->base.get_double = stats_max64_get_double;
  me->base.get_string = stats_max64_get_string;
  me->base.set_double = stats_max64_set_double;
  me->base.set_string = stats_max64_set_string;
  me->base.close = stats_max64_close;

  if (cfg != 0) {
    stats_config((stats_t*)me,cfg);
  }

  return (stats_t*) me;
}

