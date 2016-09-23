#include "rng_rdrand.h"
#include <string.h>
#include <immintrin.h>

#define PAGE 4096

typedef struct
{
  reader_t base;
  uint8_t data[PAGE];
  size_t size;
  size_t at;
} rng_rdrand_t;

static ssize_t rng_rdrand_read(reader_t *me, uint8_t *buffer, size_t size)
{
  size_t remain = size;
  
  rng_rdrand_t *ME = (rng_rdrand_t*)me;
  while (remain > 0) {
    if (ME->at >= PAGE) {
      {int i; for (i = 0; i<PAGE; i += 8)  {
        _rdrand64_step((uint64_t*)&(ME->data[i]));
        }}
      ME->at = 0;
    }
    ssize_t n=(PAGE)-(ME->at);
    if (n > remain) {
      n = remain;
    }
    memcpy(buffer,ME->data+ME->at,n);
    ME->at += n;
    remain   -= n;
    buffer += n;
  }
  return size;
}

static void rng_rdrand_close(reader_t *me)
{
  rng_rdrand_t *ME = (rng_rdrand_t*)me;  
  free(ME);
}

reader_t* rng_rdrand(const char *config)
{
  rng_rdrand_t *me =
    (rng_rdrand_t*)malloc(sizeof(rng_rdrand_t));
  
  if (me == 0) return (reader_t*)0;

  me->at=PAGE;
  me->base.read=rng_rdrand_read;
  me->base.close=rng_rdrand_close;
  
  return (reader_t*)me;
}
