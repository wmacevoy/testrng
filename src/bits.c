#include "bits.h"

#include <stdlib.h>
#include <string.h>

#define BITS 0x10000

typedef struct
{
  bits_t base;
  reader_t *source;
  uint8_t data[BITS/8];
  int at;
  uint8_t flags;
} bits_impl_t;

static int bits_impl_read(struct bits *me, void *dst, int offset, int length)
{
  int bits = 0;
  bits_impl_t *ME = (bits_impl_t*)me;
  uint8_t *data = ME->data;
  int at = ME->at;

  while (length > 0) {
    if (at >= BITS) {
      ssize_t got=reader_read(ME->source,data,BITS/8);
      if (got < BITS/8) {
        if (got <= 0) {
          ME->at=BITS;
          return bits;
        }
        memcpy(data+(BITS/8)-got,data,got);
        at = BITS-got*8;
      } else {
        at = 0;
      }
    }
    int n = BITS - at;
    if (n > length)  n = length;
    if (dst != 0) {int i; for (i=0; i<n; ++i) {
        if (data[(at+i)/8] & (1 << (at+i)%8)) {
          ((char*)dst)[(offset+i)/8] |= (1 << ((offset+i) % 8));
        } else {
          ((char*)dst)[(offset+i)/8] &= ~(1 << ((offset+i) % 8));
        }
      }}
    offset += n;
    at += n;
    length -= n;
    bits += n;
  }
  ME->at = at;
  return bits;
}

static void bits_impl_close(bits_t *me)
{
  bits_impl_t *ME = (bits_impl_t*)me;
  if ((ME->flags & BITS_CLOSE) != 0) {
    ME->source->close(ME->source);
  }
  free(ME);
}

bits_t *bits(reader_t *source, uint8_t flags)
{
  bits_impl_t *me =
    (bits_impl_t*)malloc(sizeof(bits_impl_t));
  
  if (me == 0) return (bits_t*)0;

  me->source = source;
  me->at  = BITS;
  me->flags = flags;
  me->base.read=bits_impl_read;
  me->base.close=bits_impl_close;
  
  return (bits_t*)me;
}
