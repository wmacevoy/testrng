#pragma once

#include "reader.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct stats 
  {
    void (*config)(struct stats *me, const char *args);
    void (*run)(struct stats *me, reader_t *src);
    int (*get_double_index)(struct stats *me, const char *name);
    int (*get_string_index)(struct stats *me, const char *name);
    double (*get_double)(struct stats *me, int index);
    char * (*get_string)(struct stats *me, int index);
    int (*set_double)(struct stats *me, int index, double value);
    int (*set_string)(struct stats *me, int index, const char *value);
    void (*close)(struct stats *me);
  } stats_t;

  static inline void stats_config(stats_t *h, const char *cfg) {
    h->config(h,cfg);
  }

  static inline void stats_run(stats_t *h, reader_t *src) {
    h->run(h,src);
  }

  static inline int stats_get_double_index(stats_t *h, const char *name) {
    return h->get_double_index(h,name);
  }

  static inline double stats_get_double_by_name(stats_t *h, const char *name) {
    return h->get_double(h,h->get_double_index(h,name));
  }

  static inline int stats_set_double_by_name(stats_t *h, const char *name, double value) {
    return h->set_double(h,h->get_double_index(h,name),value);
  }

  static inline double stats_get_double(stats_t *h, int index) {
    return h->get_double(h,index);
  }

  static inline int stats_set_double(stats_t *h, int index, double value) {
    return h->set_double(h,index,value);
  }

  static inline int stats_get_string_index(stats_t *h, const char *name) {
    return h->get_string_index(h,name);
  }

  static inline char* stats_get_string_by_name(stats_t *h, const char *name) {
    return h->get_string(h,h->get_string_index(h,name));
  }

  static inline int stats_set_string_by_name(stats_t *h, const char *name, char *value) {
    return h->set_string(h,h->get_string_index(h,name),value);
  }

  static inline char *stats_get_string(stats_t *h, int index) {
    return h->get_string(h,index);
  }

  static inline int stats_set_string(stats_t *h, int index, const char *value) {
    return h->set_string(h,index,value);
  }

  static inline void stats_close(stats_t *h) {
    h->close(h);
  }

#ifdef __cplusplus
} /* extern "C" */
#endif
