/** \file
 *  \brief  AES host interface for VSOM Platform
 *
 *  \date   November 2019
 *
 *  \copyright Honeywell
 *  ALL RIGHTS RESERVED, Honeywell Confidential and Proprietary.
 */

#ifndef __AES_HOST_H__
#define __AES_HOST_H__

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "fsl_common.h"

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
#include "mbedtls/memory_buffer_alloc.h"
#endif

#include "mbedtls/md.h"
#include "mbedtls/aes.h"
#include "mbedtls/cipher.h"


#ifdef __cplusplus
extern "C" {
#endif
  
#define RT_DCP_USE_OTP_KEY 1 /* Set to 1 to select OTP key for AES encryption/decryption. */
#if RT_DCP_USE_OTP_KEY
#define DCP_KEY         kDCP_OTPMKKeyHigh
#else
#define DCP_KEY         kDCP_None    
#endif
  
#define RT_AES_BLOCK_SIZE   (16u) // 16 bytes
#define RT_AES_IV_SIZE       16
  
enum _aes_status
{
    kStatusInvalidLENGTH = MAKE_STATUS(kStatusGroup_DCP, 1),
    kStatusInvalidALLOC = MAKE_STATUS(kStatusGroup_DCP, 2),
    kStatusInvalidUNAVAILABLE = MAKE_STATUS(kStatusGroup_DCP, 3),
    kStatusInvalidINTERNAL = MAKE_STATUS(kStatusGroup_DCP, 4),
};
  
typedef enum
{
    RT_AES_MODE_CBC,
    RT_AES_MODE_CBC_PAD_PCKS7,
    RT_AES_MODE_CFB,
    RT_AES_MODE_CTR,
    RT_AES_MODE_ECB,
    RT_AES_MODE_ECB_PAD_PCKS7,
} rt_aes_mode_t;
  
typedef enum
{
    RT_DECRYPT       = 0,
    RT_ENCRYPT       = 1,
    RT_MAC_CALCULATE = 2
} rt_operation_t;

typedef enum
{
    RT_BITS_128       = 128,
    RT_BITS_192       = 192,
    RT_BITS_256       = 256
} rt_keybits_t;


  
typedef struct
{
    rt_aes_mode_t mode;
    rt_operation_t operation;
    rt_keybits_t key_size;
    uint8_t iv[RT_AES_IV_SIZE];
    mbedtls_aes_context ctx;
} rt_aes_context_t;


typedef enum _dcp_otp_key_select
{
    kDCP_None,
    kDCP_OTPMKKeyLow  = 1U, /* Use [127:0] from snvs key as dcp key */
    kDCP_OTPMKKeyHigh = 2U, /* Use [255:128] from snvs key as dcp key */
    kDCP_OCOTPKeyLow  = 3U, /* Use [127:0] from ocotp key as dcp key */
    kDCP_OCOTPKeyHigh = 4U  /* Use [255:128] from ocotp key as dcp key */
} dcp_otp_key_select;


void reverse_memcpy(const uint8_t* src, uint8_t* dst, uint32_t size);

status_t DCP_OTPKeySelect(dcp_otp_key_select keySelect);

status_t rt_aes_ctr_construct_iv(uint8_t * iv, size_t counter);

status_t rt_aes_init(void * const    p_context,
                     rt_aes_mode_t   mode,
                     rt_operation_t  operation);
  
status_t rt_aes_key_set(void * const p_context, uint8_t * p_key);

status_t rt_aes_iv_set(void * const p_context, uint8_t * p_iv);

status_t rt_aes_update(void * const p_context,
                       uint8_t * p_data_in,
                       size_t    data_size,
                       uint8_t * p_data_out);

status_t rt_aes_finalize(void *         const p_context,
                         uint8_t *      p_data_in,
                         size_t         data_size,
                         uint8_t *      p_data_out,
                         size_t *       p_data_out_size);

status_t rt_aes_crypt(rt_aes_mode_t   mode,
                      rt_operation_t  operation,
                      uint8_t *       p_key,
                      uint8_t *       p_iv,
                      uint8_t *       p_data_in,
                      size_t          data_size,
                      uint8_t *       p_data_out,
                      size_t *        p_data_out_size);

void BEE_config(uint32_t start, uint32_t end, uint32_t* counter);

status_t rt_aes_ctr(dcp_otp_key_select keySelect,
                        uint8_t *       p_key,
                        uint8_t *       p_iv,
                        uint8_t *       p_data_in,
                        size_t          data_size,
                        uint8_t *       p_data_out);

status_t rt_get_prdb_ctr(uint8_t idx, uint8_t* counter);

status_t rt_aes_test(void);

status_t RT_Dec_Verify_Data(const uint8_t *in_cipher, uint32_t in_length, uint8_t  *out_plaintext, uint32_t *out_length, bool en_integrity);

status_t RT_Enc_Hash_Data(const uint8_t *in_plaintext, uint32_t in_length, uint8_t *out_cipher, uint32_t *out_length, bool en_integrity);

status_t RT_Enc_Dec_Verify_Data_Test(void);

#ifdef __cplusplus
}
#endif

#endif
