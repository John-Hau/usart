#ifndef __ECC_HOST_H__
#define __ECC_HOST_H__

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "fsl_common.h"

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
#include "mbedtls/memory_buffer_alloc.h"
#endif


#include "mbedtls/bignum.h"
#include "mbedtls/ecp.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RT_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE  (256 / 8)   
#define RT_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE   (2 * 256 / 8)
#define RT_ECC_SECP256R1_SIGNATURE_SIZE  (2 * 256 / 8)
typedef uint8_t rt_secp256r1_signature_t  [RT_ECC_SECP256R1_SIGNATURE_SIZE];
  

enum _ecc_status
{
    kStatusInvalidSignature = MAKE_STATUS(kStatusGroup_DCP, 5),
    kStatusInvalidInternal = MAKE_STATUS(kStatusGroup_DCP, 6),
};
  
typedef struct
{
    mbedtls_mpi key;                             /**< @internal @brief mbed TLS specific key representation */
} rt_ecc_private_key_t;

typedef struct
{
    mbedtls_ecp_point key;                       /**< @internal @brief mbed TLS specific key representation */
} rt_ecc_public_key_t;


int rt_ecc_mbedtls_rng(void * p_param, unsigned char * p_data, size_t size);

status_t rt_secp256r1_gen_keypair(void * p_private_key, void * p_public_key);

status_t rt_secp256r1_private_key_free(void * p_private_key);

status_t rt_secp256r1_public_key_free(void * p_public_key);

status_t rt_secp256r1_public_key_from_raw(void * p_public_key, 
                                          uint8_t const * p_raw_data,
                                          size_t raw_data_size);

#ifdef __cplusplus
}
#endif

#endif

