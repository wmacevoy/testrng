#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#ifndef LITTLE_ENDIAN
#include <byteswap.h>
#endif


#define PAGE (1024*1024)
#define BITS 31
// #define LITTLE_ENDIAN ((char*)((uint32_t)0xFF))[0]==0xFF)

const char *magic="#=======";

ssize_t bytes;
char io[PAGE];

int is_ascii() {
  int magic_len=strlen(magic);
  bytes = read(0,io,magic_len);
  return (bytes >= magic_len && strncmp(io,magic,magic_len)==0); 
}

void ascii() {
  char *line = 0;
  size_t n = 0;
  uint32_t in;
  uint64_t out;
  int bits = 0;
  for (;;) {
    if (getline(&line,&n,stdin) < 0) break;
    if (sscanf(line,"%" SCNu32 ,&in) == 1) {
#ifndef LITTLE_ENDIAN
      in=__bswap_32 (in);
#endif
      out = (out << BITS) | (in&((~((uint32_t)0))>>(32-BITS)));
      bits += BITS;
      if (bits >= 32) {
        uint32_t o32 = (out >> (bits-32));
#ifndef LITTLE_ENDIAN
        o32=__bswap_32 (out);
#endif
        write(1,&o32,4);
        bits -= 32;
      }
    }
  }

  while (bits >= 8) {
    uint8_t o8 = (out >> (bits-8));
    write(1,&o8,1);
    bits -= 8;
  }
}

void binary() {
  if (bytes > 0 && bytes < PAGE) {
    ssize_t added = read(0,io+bytes,PAGE);
    if (added > 0) bytes += added;
  }
  if (bytes > 0) write(1,io,bytes);
  while ((bytes = read(0,io,PAGE)) > 0) {
    write(1,io,bytes);
  }
}

int main()
{
  if (is_ascii()) { ascii(); }
  else { binary(); }
  return 0;
}
