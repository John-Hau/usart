
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "sha256.h"
//#include "tap.c/tap.h"

// "hello"
unsigned char hello_hashed[] = {
  0x2c, 0xf2, 0x4d, 0xba, 0x5f, 0xb0, 0xa3, 0x0e, 0x26, 0xe8, 0x3b, 0x2a,
  0xc5, 0xb9, 0xe2, 0x9e, 0x1b, 0x16, 0x1e, 0x5c, 0x1f, 0xa7, 0x42, 0x5e,
  0x73, 0x04, 0x33, 0x62, 0x93, 0x8b, 0x98, 0x24
};

// ""
unsigned char empty_hashed[] = {
  0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8,
  0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
  0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55
};




char message[] =
        "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
        "Get thee back into the tempest and the Nights Plutonian shore!"
        "Leave no black plume as a token of that lie thy soul hath spoken!"
        "Leave my loneliness unbroken! quit the bust above my door!"
        "Take thy beak from out my heart, and take thy form from off my door!"
        "Quoth the raven, Nevermore.  ";

    /* Expected SHA-256 for the message. */
uint8_t sha256[] = {0x63, 0x76, 0xea, 0xcc, 0xc9, 0xa2, 0xc0, 0x43, 0xf4, 0xfb, 0x01,
                    0x34, 0x69, 0xb3, 0x0c, 0xf5, 0x28, 0x63, 0x5c, 0xfa, 0xa5, 0x65,
                    0x60, 0xef, 0x59, 0x7b, 0xd9, 0x1c, 0xac, 0xaa, 0x31, 0xf7};



int main (void) {
 // plan(2);

  unsigned char buf[1024*8] = {0};

  //sha256_hash(buf, (unsigned char*)"hello", 5);
#if 0
void sha256_hash(unsigned char *buf, const unsigned char *data, size_t size)
{
  sha256_t hash;
  sha256_init(&hash);
  sha256_update(&hash, data, size);
  sha256_final(&hash, buf);
}
#endif

  sha256_t hash;
  sha256_init(&hash);
  sha256_update(&hash, (unsigned char*)message, strlen(message));
  sha256_final(&hash, buf);

//sha256_hash(buf, (unsigned char*)message, strlen(message));

  for(int i=0;i<32;i++)
	  printf("%2x\n",buf[i]);



  return 0;
#if 0
  note("basic stuff");
  ok(memcmp(buf, hello_hashed, 32) == 0, "should hash correctly");

  sha256_t hash;
  sha256_init(&hash);
  sha256_update(&hash, (unsigned char*)"", 0);
  sha256_update(&hash, (unsigned char*)"", 0);
  sha256_final(&hash, buf);

  ok(memcmp(buf, empty_hashed, 32) == 0, "empty should hash correctly");

  done_testing();
#endif
}
