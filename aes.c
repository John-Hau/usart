/**
 * Compile: gcc -lmcrypt aes.c -o aes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mcrypt.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "sha256.h"


#if 0
void TestAesEcb(void)
{
    static const uint8_t keyAes128[] __attribute__((aligned)) = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                                                 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};

    static const uint8_t plainAes128[]                        = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                                          0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};



    static const uint8_t cipherAes128[]                       = {0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
                                           0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97};



#if DCP_TEST_USE_OTP_KEY
#warning Please update cipherAes128 variables to match expected AES ciphertext for your OTP key.
#endif

    AT_NONCACHEABLE_SECTION_INIT(static uint8_t cipher[16]);
    AT_NONCACHEABLE_SECTION_INIT(static uint8_t output[16]);
    status_t status;

    dcp_handle_t m_handle;

    m_handle.channel    = kDCP_Channel0;
    m_handle.swapConfig = kDCP_NoSwap;

#if DCP_TEST_USE_OTP_KEY
    m_handle.keySlot = kDCP_OtpKey;
#else
    m_handle.keySlot = kDCP_KeySlot0;
#endif

    status = DCP_AES_SetKey(DCP, &m_handle, keyAes128, 16);
    TEST_ASSERT(kStatus_Success == status);

    DCP_AES_EncryptEcb(DCP, &m_handle, plainAes128, cipher, 16);
    TEST_ASSERT(memcmp(cipher, cipherAes128, 16) == 0);

    DCP_AES_DecryptEcb(DCP, &m_handle, cipher, output, 16);
    TEST_ASSERT(memcmp(output, plainAes128, 16) == 0);

    PRINTF("AES ECB Test pass\r\n");
}

void TestAesCbc(void)
{
    static const uint8_t keyAes128[] __attribute__((aligned)) = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                                                 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    static const uint8_t plainAes128[]                        = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                                          0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
    static const uint8_t ive[]                                = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

    static const uint8_t cipherAes128[] = {0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
                                           0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d};
#if DCP_TEST_USE_OTP_KEY
#warning Please update cipherAes128 variables to match expected AES ciphertext for your OTP key.
#endif

    AT_NONCACHEABLE_SECTION_INIT(static uint8_t cipher[16]);
    AT_NONCACHEABLE_SECTION_INIT(static uint8_t output[16]);
    status_t status;

    dcp_handle_t m_handle;

    m_handle.channel    = kDCP_Channel0;
    m_handle.swapConfig = kDCP_NoSwap;

#if DCP_TEST_USE_OTP_KEY
    m_handle.keySlot = kDCP_OtpKey;
#else
    m_handle.keySlot = kDCP_KeySlot0;
#endif

    status = DCP_AES_SetKey(DCP, &m_handle, keyAes128, 16);
    TEST_ASSERT(kStatus_Success == status);

    DCP_AES_EncryptCbc(DCP, &m_handle, plainAes128, cipher, 16, ive);
    TEST_ASSERT(memcmp(cipher, cipherAes128, 16) == 0);

    DCP_AES_DecryptCbc(DCP, &m_handle, cipher, output, 16, ive);
    TEST_ASSERT(memcmp(output, plainAes128, 16) == 0);

    PRINTF("AES CBC Test pass\r\n");
}

void TestSha1(void)
{
    status_t status;
    size_t outLength;
    unsigned int length;
    unsigned char output[20];

    static const uint8_t message[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

    /* Expected SHA-1 for the message. */
    static const uint8_t sha1[] = {0x84, 0x98, 0x3e, 0x44, 0x1c, 0x3b, 0xd2, 0x6e, 0xba, 0xae,
                                   0x4a, 0xa1, 0xf9, 0x51, 0x29, 0xe5, 0xe5, 0x46, 0x70, 0xf1};

    dcp_handle_t m_handle;

    m_handle.channel    = kDCP_Channel0;
    m_handle.keySlot    = kDCP_KeySlot0;
    m_handle.swapConfig = kDCP_NoSwap;

    length    = sizeof(message) - 1;
    outLength = sizeof(output);
    memset(&output, 0, outLength);

    /************************ SHA-1 **************************/
    status = DCP_HASH(DCP, &m_handle, kDCP_Sha1, message, length, output, &outLength);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(outLength == 20u);
    TEST_ASSERT(memcmp(output, sha1, outLength) == 0);

    PRINTF("SHA-1 Test pass\r\n");
}

void TestSha256(void)
{
    status_t status;
    size_t outLength;
    unsigned int length;
    unsigned char output[32];

    static const uint8_t message[] =
        "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
        "Get thee back into the tempest and the Nights Plutonian shore!"
        "Leave no black plume as a token of that lie thy soul hath spoken!"
        "Leave my loneliness unbroken! quit the bust above my door!"
        "Take thy beak from out my heart, and take thy form from off my door!"
        "Quoth the raven, Nevermore.  ";

    /* Expected SHA-256 for the message. */
    static const unsigned char sha256[] = {0x63, 0x76, 0xea, 0xcc, 0xc9, 0xa2, 0xc0, 0x43, 0xf4, 0xfb, 0x01,
                                           0x34, 0x69, 0xb3, 0x0c, 0xf5, 0x28, 0x63, 0x5c, 0xfa, 0xa5, 0x65,
                                           0x60, 0xef, 0x59, 0x7b, 0xd9, 0x1c, 0xac, 0xaa, 0x31, 0xf7};

    dcp_handle_t m_handle;

    m_handle.channel    = kDCP_Channel0;
    m_handle.keySlot    = kDCP_KeySlot0;
    m_handle.swapConfig = kDCP_NoSwap;

    length    = sizeof(message) - 1;
    outLength = sizeof(output);
    memset(&output, 0, outLength);

    /************************ SHA-256 **************************/
    status = DCP_HASH(DCP, &m_handle, kDCP_Sha256, message, length, output, &outLength);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(outLength == 32u);
    TEST_ASSERT(memcmp(output, sha256, outLength) == 0);

    PRINTF("SHA-256 Test pass\r\n");
}
char* pad_str(char *text) {

  int size = strlen(text);
  int x = size % 16;
  int padlength = 16 - x;
  int newLength = size + padlength;

  char *newText;
  newText = malloc(newLength);

  strncpy(newText, text, newLength);

  for(int x = size; x < newLength; x++) {
    newText[x] = ' ';
  }


  return newText;
}
#endif

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























int main()
{
uint8_t keyAes128[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                       0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,0x00};

uint8_t ive[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};


uint8_t plainAes128[] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                         0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,0x00};



uint8_t cipherAes128[] = {0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
                          0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97,};


  unsigned char buf[1024*8] = {0};
  sha256_t hash;
  sha256_init(&hash);
  sha256_update(&hash, (unsigned char*)message, strlen(message));
  sha256_final(&hash, buf);


  

  //sha256_hash(buf, (unsigned char*)message, strlen(message));

  for(int i=0;i<32;i++)
	  printf("%2x ",buf[i]);

printf("\n===================================================\n");
  //char * plaintext = "test"; // Needs to be padded to 16
  //char* IV = "0123456789abcdef"; // Needs to be 16
  //char *key = "fedcba9876543210"; // Needs to be padded to 16
  //
  //
  //

  char * plaintext = plainAes128; // Needs to be padded to 16
				  //char * IV = NULL; // Needs to be 16
  char * IV = ive; // Needs to be 16
  char  *key = keyAes128; // Needs to be padded to 16

  //plaintext = pad_str(plaintext);


  int keysize = 16; // 128 bits 
  char* buffer;
  int buffer_len = 16;


  buffer = calloc(1, buffer_len);
  strncpy(buffer, plaintext, buffer_len);

  MCRYPT td = mcrypt_module_open("rijndael-128", NULL, "cbc", NULL);
  //MCRYPT td = mcrypt_module_open("rijndael-128", NULL, "ecb", NULL);
  int blocksize = mcrypt_enc_get_block_size(td);

  printf("aes128 block size is %d\n",blocksize);

  if( buffer_len % blocksize != 0 ){return 1;}

  mcrypt_generic_init(td, key, keysize, IV);


  mcrypt_generic(td, buffer, buffer_len);  // Also mdecrypt_generic.
  mcrypt_generic_deinit (td);
  mcrypt_module_close(td);

  unsigned char display;

  printf("HEX Encrypted:");
  for (int x=0; x<buffer_len; x++){
	  display = buffer[x];
	  printf("%02X", display);
  }
  printf("\n");

  return 0;
}
