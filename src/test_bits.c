#include "reader.h"
#include "bits.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

uint8_t buf[] = { 0x5A, 0x0F,0xA5, 0xED };

void test_bits1() 
{
  int n = 1000000;
  uint8_t* bin = (uint8_t*) malloc(n);
  uint8_t* bout1 = (uint8_t*) malloc(n);
  uint8_t* bout2 = (uint8_t*) malloc(n);
  {int i; for (i=0; i<n; ++i) { bin[i]=i; } }
  {int i; for (i=0; i<n; ++i) { bout1[i]=0; } }
  {int i; for (i=0; i<n; ++i) { bout2[i]=0; } }
  {int i=0,j=0,k=0; while (i < 8*n) {
      i += k; ++k;
      {int m; for (m=0; m<k && i < 8*n; ++m) {
          bout1[j/8] |= ((bin[i/8]>>(i%8))&1)<<(j%8);
          ++j;
          ++i;
        }}
    }}

  bits_t *b = bits(mem_reader(bin,n,MEM_READER_NO_COPY|MEM_READER_NO_FREE),BITS_CLOSE);
  {int j=0,k=0; for (;;) {
      bits_skip(b,k);
      ++k;
      int got=bits_read(b,bout2,j,k);
      if (got <= 0) break;
      j += got;
    }}
  {int i; for (i=0; i<n; ++i) { 
      assert (bout1[i]==bout2[i]); 
    } }

  free(bin);
  free(bout1);
  free(bout2);
}
  

void test_bits0() {

  uint8_t tmp[sizeof(buf)];

  bits_t *b = bits(mem_reader(buf,sizeof(buf),MEM_READER_NO_COPY|MEM_READER_NO_FREE),BITS_CLOSE);
  int out = 0;
  assert(bits_skip(b,4) == 4);
  assert(bits_read(b,tmp,out,4) == 4); out += 4;
  assert(bits_skip(b,4) == 4);
  assert(bits_read(b,tmp,out,4) == 4); out += 4;
  assert(bits_skip(b,4) == 4);
  assert(bits_read(b,tmp,out,4) == 4); out += 4;
  assert(bits_skip(b,4) == 4);
  assert(bits_read(b,tmp,out,4) == 4); out += 4;
  assert(bits_skip(b,4) == 0);

  assert(tmp[0] == 0x05);
  assert(tmp[1] == 0xEA);

  bits_close(b);
}

int main(int argc, char *argv[])
{
  test_bits0();
  test_bits1();
  printf("ok\n");
}
