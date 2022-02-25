/** \file
 *  \brief  Hash host interface for VSOM Platform
 *
 *  \date   November 2019
 *
 *  \copyright Honeywell
 *  ALL RIGHTS RESERVED, Honeywell Confidential and Proprietary.
 */
#ifndef __SHA256_HOST_H__
#define __SHA256_HOST_H__

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "fsl_common.h"
#ifdef FREERTOS
#include "rt_mutex_hal.h"
#endif

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
#include "mbedtls/memory_buffer_alloc.h"
#endif

#include "mbedtls/sha256.h"

#ifdef __cplusplus
extern "C" {
#endif
  
typedef struct
{
    mbedtls_sha256_context ctx;
} rt_sha256_context_t;
  

status_t rt_hash256_init(void* const p_context);

status_t rt_hash256_update(void* const p_context, uint8_t const * p_data, size_t data_size);

status_t rt_hash256_finalize(void* const p_context, uint8_t* p_digest, size_t* const p_digest_size);

status_t rt_hash256_calculate(uint8_t const * p_data, 
                                 size_t data_size, 
                                 uint8_t* p_digest, 
                                 size_t* const p_digest_size);

#ifdef __cplusplus
}
#endif

#endif

