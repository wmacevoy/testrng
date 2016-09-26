#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "parse.h"

void test_parse0() {
  parse_t *p = parse("x=3 y=5 p=bill q=(my x=3) r=(my x=(these are=()))");
  assert (parse_get_count(p) == 5);
  assert (parse_get_name(p,-1) == 0);
  assert (strcmp(parse_get_name(p,0),"x")==0);
  assert (strcmp(parse_get_name(p,1),"y")==0);
  assert (strcmp(parse_get_name(p,2),"p")==0);
  assert (strcmp(parse_get_name(p,3),"q")==0);
  assert (strcmp(parse_get_name(p,4),"r")==0);
  assert (parse_get_name(p,5) == 0);

  assert (parse_get_double(p,"x",NAN) == 3);
  assert (parse_get_double(p,"y",NAN) == 5);
  assert (isnan(parse_get_double(p,"z",NAN)));
  assert (strcmp(parse_get_string(p,"p",0),"bill")==0);
  assert (strcmp(parse_get_string(p,"q",0),"my x=3")==0);
  assert (strcmp(parse_get_string(p,"r",0),"my x=(these are=())")==0);
  assert (parse_get_string(p,"s",0) == 0);
  parse_close(p);
}

int main()
{
  test_parse0();
  printf("ok\n");
  return 0;
}
