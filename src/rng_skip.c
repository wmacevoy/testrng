#include "reader.h"
#include "bits.h"
#include "rng_load.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  reader_t base;
  bits_t *bits;
  uint32_t at;
  uint16_t skip;
  uint16_t keep;
} skip_reader_t;

#define ME ((skip_reader_t*)me)

static int rng_skip_read_bits(reader_t *me, uint8_t *buffer, int offset, int size) {
  int ans = 0;
  uint64_t tmp;
  while (offset < size) {
    while (ME->at < ME->skip) {
      int n=ME->skip-ME->at;
      if (n > 8*sizeof(tmp)) n = 8*sizeof(tmp);
      int m=ME->bits->read(ME->bits,&tmp,0,n);
      if (m <= 0) {
        return ans > 0 ? ans : -1;
      }
      ME->at += m;
    }

    int n = (ME->skip+ME->keep)-ME->at;
    if (n > size) n=size;
    int m=ME->bits->read(ME->bits,buffer,offset,n);
    if (m > 0) {
      ans += m;
      offset += m;
      ME->at += m;
    } else {
      break;
    }
    if (ME->at >= ME->skip+ME->keep) ME->at = 0;
  }
  return ans > 0 ? ans : -1;
}

static ssize_t rng_skip_read(reader_t *me, uint8_t *buffer, size_t size) {
  size *= 8;
  size_t ans = 0;
  size_t offset = 0;
  while (offset < size) {
    int n = 128*1024;
    if (size-offset < n) n=size-offset;
    ssize_t m = rng_skip_read_bits(me,buffer+(offset/8),offset%8,n);
    if (m > 0) {
      ans += m;
      offset += m;
    } else {
      break;
    }
  }
  return ans > 0 ? ans/8 : -1;
}

static void rng_skip_close(reader_t *me) {
  bits_close(ME->bits);
  free(me);
}

reader_t *rng_skip(const char *args) {
  reader_t *me = (reader_t*) malloc(sizeof(skip_reader_t));
  if (me == 0) return (reader_t*)0;

  memset(me,0,sizeof(skip_reader_t));

  ME->base.close = rng_skip_close;
  ME->base.read = rng_skip_read;

  ME->bits=0;
  ME->keep=31;
  ME->skip=1;
  ME->at=0;

  while (*args != 0) {
    {
      char name[4096];
      char svalue[4096];
      int delta=-1;
      sscanf(args," %[^\t =] = (%[^)]) %n",name,svalue,&delta);
      if (delta >= 0 && strcmp(name,"rng")==0) {
        reader_t *src = rng_load(svalue);
        if (src == 0) {
          free(me);
          return (reader_t*)0;
        }
        ME->bits = bits(src,BITS_CLOSE);
        if (ME->bits == 0) {
          reader_close(src);
          return (reader_t*)0;
        }
        args += delta;
        continue;
      }
    }
    {
      char name[4096];
      double dvalue;
      int delta=-1;
      
      sscanf(args," %[^\t =] = %lg %n",name,&dvalue,&delta);
      if (delta >= 0) {
        if (strcmp(name,"keep")==0) {
          ME->keep=dvalue;
          args += delta;
          continue;
        }
        if (strcmp(name,"skip")==0) {
          ME->skip=dvalue;
          args += delta;
          continue;
        }
      }
    }

    rng_skip_close(me);
    return (reader_t*)0;
  }
  return me;
}


