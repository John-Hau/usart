/** \file
 *  \brief  PRF host interface for VSOM Platform
 *
 *  \date   November 2019
 *
 *  \copyright Honeywell
 *  ALL RIGHTS RESERVED, Honeywell Confidential and Proprietary.
 */

#ifndef __PRF_HOST_H__
#define __PRF_HOST_H__

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
#include "mbedtls/cipher.h"
#include "mbedtls/md_internal.h"
#include "mbedtls/sha256.h"
#include "mbedtls/platform_util.h"


#ifdef __cplusplus
extern "C" {
#endif

enum _kdf_status
{
    kStatusSSLERROR = MAKE_STATUS(kStatusGroup_DCP, 8),
};


status_t rt_load_ota_key_host(uint8_t* root_key, uint8_t* out_key, uint8_t* params, uint8_t params_len);


#ifdef __cplusplus
}
#endif

#endif





