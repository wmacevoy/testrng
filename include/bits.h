#pragma once

#include "reader.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct bits
  {
    int (*read)(struct bits *me, void *dst, int offset, int length);
    void (*close)(struct bits *me);
  } bits_t;

#define BITS_CLOSE 0x01
#define BITS_NO_CLOSE 0x00

  bits_t *bits(reader_t *source, uint8_t flags);

  static inline int bits_read(bits_t *h, void *dst, int offset, int length) {
    return h->read(h,dst,offset,length);
  }

  static inline int bits_skip(bits_t *h, int length) {
    return h->read(h,0,0,length);
  }

  static inline void bits_close(bits_t *h) {
    h->close(h);
  }

#ifdef __cplusplus
} /* extern "C" */
#endif
