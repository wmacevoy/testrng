#include "reader.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct
  {
    reader_t base;
    uint8_t *data;
    size_t size;
    size_t at;
    uint8_t flags;
  } mem_reader_t;

  static ssize_t mem_reader_read(reader_t *me, uint8_t *buffer, size_t size)
  {
    mem_reader_t *ME = (mem_reader_t*)me;
    ssize_t rest=(ME->size)-(ME->at);
    if (size > rest) {
      size = rest;
    }
    memcpy(buffer,ME->data+ME->at,size);
    ME->at += size;
    return size;
  }

  static void mem_reader_close(reader_t *me)
  {
    mem_reader_t *ME = (mem_reader_t*)me;  
    if ((ME->flags & MEM_READER_FREE)) {
      free(ME->data);
    }
    free(ME);
  }

  reader_t* mem_reader(const uint8_t *data, size_t size, uint8_t flags)
  {
    mem_reader_t *me =
      (mem_reader_t*)malloc(sizeof(mem_reader_t));

    if (me == 0) return (reader_t*)0;

    if ((flags & MEM_READER_COPY) != 0) {
      me->data = (uint8_t*) malloc(size);
      if (me->data == 0) {
        free(me);
        return (reader_t*)0;
      }
    } else {
      me->data = (uint8_t*) data;
    }
    me->size = size;
    me->at=0;
    me->flags=flags;
    me->base.read=mem_reader_read;
    me->base.close=mem_reader_close;
  
    return (reader_t*)me;
  }

  typedef struct
  {
    reader_t base;
    int dev;
  } dev_reader_t;

  static ssize_t dev_reader_read(reader_t *me, uint8_t *buffer, size_t size)
  {
    dev_reader_t *ME = (dev_reader_t*)me;
    return read(ME->dev,buffer,size);
  }

  static void dev_reader_close(reader_t *me)
  {
    dev_reader_t *ME = (dev_reader_t*)me;
    close(ME->dev);
    free(me);
  }

  reader_t *dev_reader(int dev)
  {
    dev_reader_t *me = (dev_reader_t*)malloc(sizeof(dev_reader_t));
    if (me == 0) return 0;
    me->dev=dev;
    me->base.read=dev_reader_read;
    me->base.close=dev_reader_close;
    
    return (reader_t*)me;
  }

  typedef struct
  {
    reader_t base;
    FILE *file;
  } file_reader_t;


  static ssize_t file_reader_read(reader_t *me, uint8_t *buffer, size_t size)
  {
    file_reader_t *ME = (file_reader_t*)me;
    return fread(buffer,1,size,ME->file);
  }

  static void file_reader_close(reader_t *me)
  {
    file_reader_t *ME = (file_reader_t*)me;
    fclose(ME->file);
    free(me);
  }
  
  reader_t *file_reader(FILE *file)
  {
    file_reader_t *me = (file_reader_t*)malloc(sizeof(file_reader_t));
    if (me == 0) return 0;
    me->file=file;
    me->base.read=file_reader_read;
    me->base.close=file_reader_close;
    
    return (reader_t*)me;
  }

  typedef struct
  {
    reader_t base;
    int dev;
    pid_t child;
  } pipe_reader_t;

  static ssize_t pipe_reader_read(reader_t *me, uint8_t *buffer, size_t size)
  {
    pipe_reader_t *ME = (pipe_reader_t*)me;
    return read(ME->dev,buffer,size);
  }

  static void pipe_reader_close(reader_t *me)
  {
    pipe_reader_t *ME = (pipe_reader_t*)me;
    kill(ME->child, SIGTERM);
    close(ME->dev);
    waitpid(ME->child, 0, 0);
    free(me);
  }

  reader_t *pipe_reader(const char *cmd) {
    int fd[2];
    pipe(fd);
    pid_t child;

    if ((child = fork()) == 0) {
      close(1);
      dup2(fd[1],1);
      close(fd[0]);
      close(fd[1]);
      exit(system(cmd));
    } else {
      pipe_reader_t* me = (pipe_reader_t*) malloc(sizeof(pipe_reader_t));
      if (me == 0) return 0;
      me->dev=fd[0];
      me->child=child;
      close(fd[1]);
      me->base.read=pipe_reader_read;
      me->base.close=pipe_reader_close;
      return (reader_t*)me;
    }
  }

  typedef struct {
    reader_t base;
    join_reader_next_t *next;
    void *misc;
    reader_t *current;
  } join_reader_t;

  static ssize_t join_reader_read(reader_t *me, uint8_t *buffer, size_t size)
  {
    join_reader_t *ME = (join_reader_t*)me;    
    ssize_t ans = 0;
    while (ME->current != 0 && size > 0) {
      ssize_t part=ME->current->read(ME->current,buffer,size);
      if (part < 0) {
        return part;
      } else if (part == 0) {
        ME->current->close(ME->current);
        ME->current = ME->next(ME->misc);
      } else {
        buffer += part;
        size -= part;
        ans += part;
      }
    }
    return ans;
  }

  static void join_reader_close(reader_t *me)
  {
    join_reader_t *ME = (join_reader_t*)me;
    while (ME->current != 0) {
      ME->current->close(ME->current);
      ME->current=ME->next(ME->misc);
    }
    free(ME);
  }

  reader_t *join_reader(join_reader_next_t *next,void *misc)
  {
    join_reader_t *me =
      (join_reader_t*) malloc(sizeof(join_reader_t));
    if (me == 0) return 0;
    me->next=next;
    me->misc=misc;
    me->current=me->next(misc);
    me->base.read=join_reader_read;
    me->base.close=join_reader_close;
    return (reader_t*)me;
  }

  typedef struct {
    size_t size;
    ssize_t current;
    reader_t **readers;
  } readers_misc_t;

  static reader_t *readers_next(void *misc)
  {
    readers_misc_t *MISC = (readers_misc_t*) misc;
    ++MISC->current;
    if (MISC->current < MISC->size) {
      return MISC->readers[MISC->current];
    } else {
      free(MISC->readers);
      free(MISC);
      return 0;
    }
  }
  
  reader_t *readers(size_t size,...)
  {
    va_list ap;
    size_t arg;
    readers_misc_t *misc = (readers_misc_t*)malloc(sizeof(readers_misc_t));
    if (misc == 0) return 0;
    misc->size=size;
    misc->readers = (reader_t**)malloc(sizeof(reader_t*)*size);
    if (misc->readers == 0) {
      free(misc);
      return 0;
    }
    va_start(ap,size);
    for (arg=0; arg<size; ++arg) {
      misc->readers[arg]=va_arg(ap,reader_t*);
    }
    va_end(ap);
    misc->current=-1;

    reader_t *ans = join_reader(readers_next,misc);

    if (ans == 0) {
      free(misc->readers);
      free(misc);
    }

    return ans;
  }

#ifdef __cplusplus
} /* extern "C" */
#endif
