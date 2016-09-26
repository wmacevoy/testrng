#include "reader.h"
#include "bits.h"
#include "rng_load.h"
#include "parse.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE 4096

typedef struct
{
  reader_t base;
  bits_t *bits;
  uint32_t at;
  uint16_t skip;
  uint16_t keep;
} skip_reader_t;

#define ME ((skip_reader_t*)me)

static int rng_skip_read_bits(reader_t *me, uint8_t *buffer, int offset0, int size) {
  int offset = offset0;
  int offset1 = offset0+size;
  while (offset < offset1) {
    while (ME->at < ME->skip) {
      int mskip=bits_skip(ME->bits,ME->skip-ME->at);
      if (mskip <= 0) goto exit;
      ME->at += mskip;
    }
    while (ME->at < ME->skip+ME->keep) {
      int nwant = (ME->skip+ME->keep)-ME->at;
      if (nwant > size) nwant=size;
      int mkeep=bits_read(ME->bits,buffer,offset,nwant);
      if (mkeep <= 0) goto exit;
      ME->at += mkeep;
      offset += mkeep;
    }
    ME->at = 0;
  }
 exit: return offset > offset0 ? offset-offset0 : 0;
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
  parse_t *p = parse(args);
  if (p == 0) return 0;

  reader_t *me = (reader_t*) malloc(sizeof(skip_reader_t));
  if (me == 0) return (reader_t*)0;

  memset(me,0,sizeof(skip_reader_t));

  ME->base.close = rng_skip_close;
  ME->base.read = rng_skip_read;

  ME->bits=bits(rng_load(parse_get_string(p,"rng","rdrand")),BITS_CLOSE);
  ME->keep=parse_get_double(p,"keep",31);
  ME->skip=parse_get_double(p,"skip",1);
  ME->at=0;

  parse_close(p);

  return me;
}


