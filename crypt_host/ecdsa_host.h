#ifndef __ECDSA_HOST_H__
#define __ECDSA_HOST_H__


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
#include "mbedtls/ecdsa.h"

#ifdef __cplusplus
extern "C" {
#endif


status_t rt_secp256r1_ecdsa_sign(void           const * p_private_key,
                                 uint8_t         const * p_hash,
                                 size_t          hash_size,
                                 uint8_t         * p_signature,
                                 size_t          signature_size);


status_t rt_secp256r1_ecdsa_verify(void       const * p_public_key,
                                    uint8_t    const * p_hash,
                                    size_t     hash_size,
                                    uint8_t    const * p_signature,
                                    size_t     signature_size);


status_t rt_ecdsa_test(void);


#ifdef __cplusplus
}
#endif

#endif
