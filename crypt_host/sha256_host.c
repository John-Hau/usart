/** \file
 *  \brief  Hash host interface for VSOM Platform
 *
 *  \date   November 2019
 *
 *  \copyright Honeywell
 *  ALL RIGHTS RESERVED, Honeywell Confidential and Proprietary.
 */

#include "sha256_host.h"

#ifdef FREERTOS
#include "rt_mutex_hal.h"

extern rt_lock_t dcp_lock;
#endif

/**
 * \brief Initializing the context structure.
 *
 * \param[in,out] p_context       Pointer to structure holding context information
 * \return NRF_SUCCESS on success
*/
status_t rt_hash256_init(void* const p_context)
{
    rt_sha256_context_t *ctx = (rt_sha256_context_t *)p_context;
  
    mbedtls_sha256_init( &ctx->ctx );
    
    return mbedtls_sha256_starts_ret( &ctx->ctx, 0 );
}


status_t rt_hash256_update(void* const p_context, uint8_t const * p_data, size_t data_size)
{
    rt_sha256_context_t *ctx = (rt_sha256_context_t *)p_context;
    status_t ret_val;
    
    #ifdef FREERTOS
    ret_val = rt_lock_mutex(&dcp_lock);
    if(kStatus_Success != ret_val) return ret_val;
    #endif
    
    ret_val = mbedtls_sha256_update_ret( &ctx->ctx, p_data, data_size );
    
    #ifdef FREERTOS
    rt_unlock_mutex(&dcp_lock);
    #endif
    
    return ret_val;
}


status_t rt_hash256_finalize(void* const p_context, uint8_t* p_digest, size_t* const p_digest_size)
{
    rt_sha256_context_t *ctx = (rt_sha256_context_t *)p_context;
    status_t ret_val;
    
    #ifdef FREERTOS
    ret_val = rt_lock_mutex(&dcp_lock);
    if(kStatus_Success != ret_val) return ret_val;
    #endif    
    
    *p_digest_size = 32;
    ret_val = mbedtls_sha256_finish_ret( &ctx->ctx, p_digest );
    
    #ifdef FREERTOS
    rt_unlock_mutex(&dcp_lock);
    #endif
    
    return ret_val;
}


status_t rt_hash256_calculate(uint8_t const * p_data, 
                                 size_t data_size, 
                                 uint8_t* p_digest, 
                                 size_t* const p_digest_size)
{
    status_t ret_val;
    rt_sha256_context_t ctx;
    
    ret_val = rt_hash256_init(&ctx);
    if(kStatus_Success != ret_val) return ret_val;

    ret_val = rt_hash256_update(&ctx, p_data, data_size);
    if(kStatus_Success != ret_val) return ret_val;
    
    ret_val = rt_hash256_finalize(&ctx, p_digest, p_digest_size);

    return ret_val;
}





