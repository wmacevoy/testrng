#include "reader.h"
#include "bits.h"
#include "rng_load.h"
#include "parse.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef LITTLE_ENDIAN
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LITTLE_ENDIAN
#endif
#endif

#ifdef LITTLE_ENDIAN
#include <byteswap.h>
#endif

typedef struct
{
  reader_t base;
  reader_t *rng;
  uint64_t out;
  uint32_t bits;
  uint32_t keep;
} rng_keep32_t;

#define ME ((rng_keep32_t*)me)

static ssize_t rng_keep32_read(reader_t *me, uint8_t *buffer, size_t size) {
  ssize_t ans = 0;
  while (size > 0) {
    if (ME->bits <= 32) {
      uint32_t in;
      if (ME->rng->read(ME->rng,(uint8_t*)&in,4) != 4) {
	return ans;
      }
#ifndef LITTLE_ENDIAN
      in=__bswap_32 (in);
#endif
      ME->out = (ME->out << (ME->keep)) | (in&((~((uint32_t)0))>>(32-(ME->keep))));
      ME->bits += ME->keep;
    }
    if (size >= 4 && ME->bits >= 32) {
      uint32_t o32 = (ME->out >> (ME->bits-32));
#ifndef LITTLE_ENDIAN
      o32=__bswap_32 (o32);
#endif
      *(uint32_t*)buffer = o32;
      ME->bits -= 32;
      size -= 4;
      buffer += 4;
      ans += 4;
    } else if (size >= 1 && ME->bits >= 8) {
      uint8_t o8 = (ME->out >> (ME->bits-8));
      *buffer = o8;
      ME->bits -= 8;
      size -= 1;
      buffer += 1;
      ans += 1;
    }
  }
  return ans;
}

static void rng_keep32_close(reader_t *me) {
  reader_close(ME->rng);
  free(me);
}

reader_t *rng_keep32(const char *args) {
  parse_t *p = parse(args);
  if (p == 0) return 0;

  reader_t *me = (reader_t*) malloc(sizeof(rng_keep32_t));
  if (me == 0) return (reader_t*)0;

  memset(me,0,sizeof(rng_keep32_t));

  ME->base.close = rng_keep32_close;
  ME->base.read = rng_keep32_read;

  ME->rng=rng_load(parse_get_string(p,"rng","rdrand"));
  ME->keep=parse_get_double(p,"keep",31);
  ME->bits=0;

  parse_close(p);

  return me;
}
