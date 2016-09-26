#pragma once

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct parse {
    double (*get_double)(struct parse *me, const char *name, double defval);
    const char *(*get_string)(struct parse *me, const char *name, const char *defval);
    const char *(*get_name)(struct parse *me, int i);
    int (*get_count)(struct parse *me);
    void (*close)(struct parse *me);
  } parse_t;
  
  parse_t *parse(const char *args);

  static inline double parse_get_double(parse_t *me, const char *name, double defval) {
    return me->get_double(me,name,defval);
  }

  static inline const char*  parse_get_string(parse_t *me, const char *name, const char *defval) {
    return me->get_string(me,name,defval);
  }

  static inline void parse_close(parse_t *me) {
    return me->close(me);
  }

  static inline int parse_get_count(parse_t *me) {
    return me->get_count(me);
  }

  static inline const char* parse_get_name(parse_t *me, int i) {
    return me->get_name(me,i);
  }

  void parse_white(const char **args);
  char *parse_id(const char **args);
  int parse_eq(const char **args);
  char *parse_string(const char **args);
  double parse_double(const char **args);

#ifdef __cplusplus
} /* extern "C" */
#endif
