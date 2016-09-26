#include "parse.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>

typedef struct {
  char *name;
  double dvalue;
  char *svalue;
} arg_t;

typedef struct
{
  parse_t base;
  int nargs;
  arg_t *args;
} parse_impl_t;

#define ME ((parse_impl_t*)me)

static arg_t *parse_impl_find(parse_t *me, const char *name) {
  int i;
  for (i=0; i<ME->nargs; ++i) { 
    if (strcmp(name,ME->args[i].name)==0) return &ME->args[i];
  }
  return 0;
}

static double parse_impl_get_double(parse_t *me, const char *name, double defval) {
  arg_t *arg = parse_impl_find(me,name);
  return arg != 0 ? arg->dvalue : defval;
}

static const char * parse_impl_get_string(parse_t *me, const char *name, const char *defval) {
  arg_t *arg = parse_impl_find(me,name);
  return arg != 0 ? arg->svalue : defval;
}

static int parse_impl_get_count(parse_t *me) {
  return ME->nargs;
}

static const char *parse_impl_get_name(parse_t *me, int i) {
  if (0 <= i && i < ME->nargs) {
    return ME->args[i].name;
  } else {
    return 0;
  }
}


static void parse_impl_close(parse_t *me) {
  free(ME->args);
  free(ME);
}

static inline int end(const char **args) {
  return (args == 0 || *args == 0 || **args==0);
}

void parse_white(const char **args) {
  if (end(args)) return;
  while (**args != 0 && isblank(**args)) ++*args;
}

char *parse_id(const char **args) {
  if (end(args)) return strdup("");

  parse_white(args);
  const char *s=*args;
  while (**args != 0 && !isblank(**args) && **args != '=') { ++*args; }
  int n=(*args)-s;
  char *id = (char*)malloc(n+1);
  memcpy(id,s,n);
  id[n]=0;
  parse_white(args);
  return id;
}

int parse_eq(const char **args) {
  if (end(args)) return 0;

  parse_white(args);
  if (**args == '=') {
    ++*args;
    parse_white(args);
    return 1;
  } else {
    return 0;
  }
}

char* parse_string(const char **args) {
  if (end(args)) return strdup("");

  parse_white(args);
  const char *s=*args;
  if (*s != '(') {
    while (**args != 0 && !isblank(**args)) { ++*args; }
    int n=(*args)-s;
    char *ans = (char*)malloc(n+1);
    memcpy(ans,s,n);
    ans[n]=0;
    parse_white(args);
    return ans;
  } else {
    int pc=0;
    while (**args != 0) {
      if (**args == '(') ++pc;
      if (**args == ')') --pc;
      ++*args;
      if (pc == 0) break;
    }
    int n=(*args)-s-2;
    if (pc > 0) ++n;
    char *ans = (char*)malloc(n+1);
    memcpy(ans,s+1,n);
    ans[n]=0;
    parse_white(args);
    return ans;
  }
}

double parse_double(const char **args) {
  double ans = NAN;
  char *s = parse_string(args);
  if (strlen(s) != 0 && strcmp(s,"nan")!=0) {
    ans = atof(s);
  }
  free(s);
  return ans;
}

parse_t *parse(const char *args) {
  const char *args0 = args;
  parse_t *me = (parse_t*) malloc(sizeof(parse_impl_t));
  if (me == 0) return (parse_t*)0;
  memset(me,0,sizeof(parse_impl_t));
  ME->nargs=0;
  ME->args=0;
  ME->base.get_double = parse_impl_get_double;
  ME->base.get_string = parse_impl_get_string;
  ME->base.get_count = parse_impl_get_count;
  ME->base.get_name = parse_impl_get_name;
  ME->base.close = parse_impl_close;

  while (args != 0 && *args !=0) {
    arg_t arg;
    arg.name=0;
    arg.svalue=0;
    arg.dvalue=NAN;

    parse_white(&args);
    arg.name=parse_id(&args);
    int eq=parse_eq(&args);
    arg.svalue=parse_string(&args);
    arg.dvalue=atof(arg.svalue);
    parse_white(&args);
    
    if (strlen(arg.name)>0 && eq) {
      ++ME->nargs;
      ME->args = realloc(ME->args,sizeof(arg_t)*ME->nargs);
      memcpy(ME->args+(ME->nargs-1),&arg,sizeof(arg_t));
    } else {
      free(arg.name);
      free(arg.svalue);
      fprintf(stderr,"error parsing %s near %s\n",args0,args);
      parse_close(me);
      return 0;
    }
  }

  return me;
}
