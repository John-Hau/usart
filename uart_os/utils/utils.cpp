
#include "baseplate.h"
#include "utils.h"

static const char hex_str[] = "0123456789abcdef";

unsigned snprintmem(const uint8_t* memblock, unsigned size, char* buffer, unsigned buffersize) {
  return __snprintmem__(memblock, size, buffer, buffersize, hex_str);
}

unsigned rsnprintmem(const uint8_t* memblock, unsigned size, char* buffer, unsigned buffersize) {
  return __rsnprintmem__(memblock, size, buffer, buffersize, hex_str);
}
