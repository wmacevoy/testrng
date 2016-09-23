#pragma once

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct reader
  {
    ssize_t (*read)(struct reader *me, uint8_t *buffer, size_t size);
    void (*close)(struct reader *me);
  } reader_t;

#define MEM_READER_COPY 0x01
#define MEM_READER_NO_COPY 0x00
#define MEM_READER_FREE 0x02
#define MEM_READER_NO_FREE 0x00

  reader_t *mem_reader(const uint8_t *data, size_t size, uint8_t flags);

  reader_t *dev_reader(int dev);
  reader_t *file_reader(FILE *file);
  reader_t *pipe_reader(const char *cmd);

  typedef reader_t *join_reader_next_t(void *misc);

  /* repeatedly call next(misc) to obtain a sequence of readers,
     with null indicating no more remain */
  reader_t *join_reader(join_reader_next_t *next,void *misc);

  /* join_reader wrapper for var-arg list of readers */
  reader_t *readers(size_t size,...);

  static inline ssize_t reader_read(reader_t *h, uint8_t *dst, int length) {
    return h->read(h,dst,length);
  }

  static inline void reader_close(reader_t *h) {
    h->close(h);
  }

#ifdef __cplusplus
} /* extern "C" */
#endif
