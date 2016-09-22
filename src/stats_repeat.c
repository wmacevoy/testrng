#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include "stats_repeat.h"
#include "stats_load.h"

typedef struct stats_repeat {
  stats_t base;
  double samples;
  double sample;
  double limit;
  double progress;
  stats_t *stats;
  double zl;
  double df;
} stats_repeat_t;

enum StringIndexes {
  SIName=0x10000,
  SIStats,
  SIConfig,
  SIResults,
  SIEnd,
};

enum DoubleIndexes {
  DISamples,
  DISample,
  DILimit,
  DIProgress,
  DILuck,
  DIZLuck,
  DINLuck,
  DIDF,
};

#define ME ((stats_repeat_t*)me)

static void stats_repeat_config(stats_t *me, const char *args) {
  char name[4096];

  while (*args != 0) {
    {
      int delta = -1;
      double dvalue;
      sscanf(args," %[^\t =] = %lf %n",name,&dvalue,&delta);
      if (delta >= 0) {
        if (stats_set_double_by_name(me,name,dvalue)) {
          args += delta;
          continue;
        }
      }
    }

    {
      int delta = -1;
      char svalue[4096];
      sscanf(args," %[^\t =] = (%[^)]) %n",name,svalue,&delta);
      if (delta >= 0) {
        if (stats_set_string_by_name(me,name,svalue)) {
          args += delta;
          continue;
        }
      }
    }

    { 
      int delta = -1;
      char junk[4096];
      sscanf(args," %s %n",junk,&delta);
      if (delta >= 0) {
        args += delta;
      }
    }
  }
}

static double zluck(stats_repeat_t *me) {
  return ME->zl;
}

static double luck(stats_t *me) {
  if (isnan(ME->zl)) return NAN;
  return 0.5*(1+erf(ME->zl));
}

static double df(stats_t *me) {
  return ME->df;
}

static char *config(stats_t *me) {
  char tmp[4096];
  char *stats_name = stats_get_string_by_name(ME->stats,"name");
  char *stats_config = stats_get_string_by_name(ME->stats,"config");
  snprintf(tmp,sizeof(tmp),
           "samples=%lg limit=%lg progress=%lg stats=(%s %s)",
           ME->samples,ME->limit,ME->progress,
           stats_name,stats_config);

  free(stats_name);
  free(stats_config);

  return strdup(tmp);
}

static char *results(stats_t *me) {
  char tmp[4096];
  char *msg = "";
  if (!isnan(ME->limit) && !isnan(ME->zl)) {
    if (ME->zl < -fabs(ME->limit)) msg=" *** UNLUCKY ***";
    if (ME->zl >  fabs(ME->limit)) msg=" ***  LUCKY ***";
  }
  snprintf(tmp,sizeof(tmp),"sample=%lg zluck=%lg df=%lg luck=%lg%s",
           ME->sample, ME->zl,ME->df, luck(me), msg);
  return strdup(tmp);
}

static int stats_repeat_get_double_index(stats_t *me, const char *name) {
  if (strcmp(name,"samples")==0) { return DISamples; }
  if (strcmp(name,"sample")==0) { return DISample; }
  if (strcmp(name,"limit")==0) { return DILimit; }
  if (strcmp(name,"progress")==0) { return DIProgress; }
  if (strcmp(name,"luck")==0) { return DILuck; }
  if (strcmp(name,"nluck")==0) { return DINLuck; }
  if (strcmp(name,"zluck")==0) { return DIZLuck; }
  if (strcmp(name,"df")==0) { return DIDF; }
  return -1;
}

static int stats_repeat_get_string_index(stats_t *me, const char *name) {
  if (strcmp(name,"name")==0) { return SIName; }
  if (strcmp(name,"stats")==0) { return SIStats; }
  if (strcmp(name,"config")==0) { return SIConfig; }
  if (strcmp(name,"results")==0) { return SIResults; }
  return -1;
}

static double stats_repeat_get_double(stats_t *me, int index) {
  switch(index) {
  case DISamples: return ME->samples;
  case DISample: return ME->sample;
  case DILimit: return ME->limit;
  case DIProgress: return ME->progress;
  case DIZLuck: return ME->zl;
  case DILuck: return luck(me);
  case DINLuck: return luck(me);
  case DIDF: return ME->df;
  }
  return NAN;
}

static int stats_repeat_set_double(stats_t *me, int index, double value) {
  switch(index) {
  case DISamples: ME->samples=value; return 1;
  case DILimit: ME->limit=value; return 1;
  case DIProgress: ME->progress=value; return 1;
  }
  return 0;
}

static char *stats_repeat_get_string(stats_t *me, int index) {
  switch(index) {
  case SIName: return strdup("repeat");
  case SIConfig: return config(me);
  case SIResults: return results(me);
  }
  return 0;
}

static int stats_repeat_set_string(stats_t *me, int index, const char *value) {
  switch(index) {
  case SIStats:
    {
      if (ME->stats != 0) {
        stats_close(ME->stats);
        ME->stats = 0;
      }

      if (value != 0 && strcmp(value,"") != 0) {
        ME->stats = stats_load(value);
      }
      return 1;
    }
  }
  return 0;
}

static void stats_repeat_run(stats_t *me, reader_t *src) {
  ME->df=0;
  ME->zl=NAN;
  int SubIndexDF = stats_get_double_index(ME->stats,"df");
  int SubIndexZL = stats_get_double_index(ME->stats,"zluck");
  for (ME->sample=0; isnan(ME->samples) || ME->sample < ME->samples; ++ME->sample) {
    stats_run(ME->stats,src);
    double df = stats_get_double(ME->stats,SubIndexDF);
    double zl = stats_get_double(ME->stats,SubIndexZL);
    if (ME->df == 0) {
      ME->df = df;
      ME->zl = zl;
    } else {
      ME->zl = sqrt(pow(ME->zl+sqrt(ME->df-0.5),2)+pow(zl+sqrt(df-0.5),2))
        -sqrt(ME->df+df-0.5);
      ME->df += df;
    }
    if (!isnan(ME->progress)) {
      if ((((uint64_t)ME->sample) % ((uint64_t)(ME->progress))) == 0) {
        printf("sample=%lg zl=%lg df=%lg\n",
               ME->sample,ME->zl,ME->df);
      }
    }
    if (!isnan(ME->limit) && (fabs(ME->zl) >= fabs(ME->limit))) {
      break;
    }
  }
}

static void stats_repeat_close(stats_t *me) {
  stats_close(ME->stats);
  free(ME);
}

stats_t* stats_repeat(const char *cfg)
{
  stats_t *me =
    (stats_t*)malloc(sizeof(stats_repeat_t));
  if (me == 0) return (stats_t*)0;
  memset(me,0,sizeof(stats_repeat_t));

  ME->samples = 1e6;
  ME->zl=NAN;
  ME->df=0;
  ME->limit=10;
  ME->progress=100000;

  ME->base.config = stats_repeat_config;
  ME->base.run = stats_repeat_run;
  ME->base.get_double_index = stats_repeat_get_double_index;
  ME->base.get_string_index = stats_repeat_get_string_index;
  ME->base.get_double = stats_repeat_get_double;
  ME->base.get_string = stats_repeat_get_string;
  ME->base.set_double = stats_repeat_set_double;
  ME->base.set_string = stats_repeat_set_string;
  ME->base.close = stats_repeat_close;

  if (cfg != 0) {
    stats_config(me,cfg);
  }

  return me;
}

