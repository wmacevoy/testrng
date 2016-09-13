#include "reader.h"

#include <string.h>
#include <assert.h>

uint8_t buf[] = { 0x00, 0x03, 0x07, 0x09 };

void test_mem_reader()
{
  uint8_t tmp[sizeof(buf)];
  memset(tmp,0,sizeof(tmp));
  reader_t *r = mem_reader(buf,sizeof(buf),MEM_READER_NO_COPY|MEM_READER_NO_FREE);
  assert(reader_read(r,tmp,1) == 1 && tmp[0] == buf[0]);
  assert(reader_read(r,tmp+1,2) == 2 && tmp[1] == buf[1] && tmp[2] == buf[2]);
  assert(reader_read(r,tmp+3,5) == 1 && tmp[3] == buf[3]);
  reader_close(r);
}

void test_file_reader()
{
  uint8_t tmp[sizeof(buf)];
  memset(tmp,0,sizeof(tmp));
  
  FILE *fout = fopen("tmp.dat","wb");
  fwrite(buf,sizeof(buf),1,fout);
  fclose(fout);

  FILE *fin = fopen("tmp.dat","rb");

  reader_t *r = file_reader(fin);
  assert(reader_read(r,tmp,1) == 1 && tmp[0] == buf[0]);
  assert(reader_read(r,tmp+1,2) == 2 && tmp[1] == buf[1] && tmp[2] == buf[2]);
  assert(reader_read(r,tmp+3,5) == 1 && tmp[3] == buf[3]);
  reader_close(r);
}

void test_pipe_reader() {
  reader_t *r = pipe_reader("/bin/bash -c 'echo abc'");
  uint8_t tmp[16];
  assert (reader_read(r,tmp,16) == 4);
  tmp[4]=0;
  assert (strcmp((const char *)tmp,"abc\n") == 0);
  reader_close(r);  
}


void test_readers()
{
  uint8_t tmp[16];
  reader_t *r = readers(2,
                        mem_reader((uint8_t*)"one",4,MEM_READER_NO_COPY|MEM_READER_NO_FREE),
                        mem_reader((uint8_t*)"two",3,MEM_READER_NO_COPY|MEM_READER_NO_FREE));
  assert(reader_read(r,tmp,sizeof(tmp)) == 7);
  assert(memcmp(tmp,"one\0two",7) == 0);
  reader_close(r);
}

int main(int argc, char *argv[])
{
  test_mem_reader();
  test_file_reader();
  test_pipe_reader();
  test_readers();
  printf("ok\n");
}
