/**
 * @file hal_aes_imxrt.c
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief AES HAL implementation for i.MX RT
 * @date 2020-05-18
 *
 * This module implements AES functions on i.MX RT using DCP periphery.
 *
 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */
#include <stddef.h>
#include "mcrypto.h"
#include "hal_aes.h"
   
#ifdef FREERTOS
#include "rt_mutex_hal.h"

extern rt_lock_t dcp_lock;
#endif

#define DCP_DEVICE DCP

mcrypto_status_t mcrypto_aes_ctx_init_with_key(mcrypto_aes_ctx_t* ctx, const mcrypto_secret_key_t* key)
{
    if ((NULL == ctx) || (NULL == key))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    mcrypto_aes_ctx_cleanup(ctx);

    ctx->dcp_handle.channel = kDCP_Channel1;
    ctx->dcp_handle.swapConfig = kDCP_NoSwap;
    ctx->dcp_handle.keySlot = kDCP_PayloadKey;

    #ifdef FREERTOS
    status_t ret_val = rt_lock_mutex(&dcp_lock);
    if(kStatus_Success != ret_val) return ret_val;
    #endif
    if (kStatus_Success != DCP_AES_SetKey(DCP_DEVICE, &(ctx->dcp_handle), (uint8_t*)(key->data), sizeof(key->data)))
    {
        #ifdef FREERTOS
        ret_val = rt_unlock_mutex(&dcp_lock);
        if(kStatus_Success != ret_val) return ret_val;
        #endif
        mcrypto_aes_ctx_cleanup(ctx);
        return MCRYPTO_FAILURE;
    }
    #ifdef FREERTOS
    ret_val = rt_unlock_mutex(&dcp_lock);
    if(kStatus_Success != ret_val) return ret_val;
    #endif
    ctx->initialized = true;

    return MCRYPTO_OK;
}

mcrypto_status_t mcrypto_aes_ctx_init_with_devkey(mcrypto_aes_ctx_t* ctx)
{
    if (NULL == ctx)
    {
        return MCRYPTO_INVALID_ARGS;
    }

    mcrypto_aes_ctx_cleanup(ctx);

    ctx->dcp_handle.channel = kDCP_Channel1;
    ctx->dcp_handle.swapConfig = kDCP_NoSwap;
    ctx->dcp_handle.keySlot = kDCP_OtpKey;

    #ifdef FREERTOS
    status_t ret_val = rt_lock_mutex(&dcp_lock);
    if(kStatus_Success != ret_val) return ret_val;
    #endif
    if (kStatus_Success != DCP_AES_SetKey(DCP_DEVICE, &(ctx->dcp_handle), NULL, 0))
    {
        mcrypto_aes_ctx_cleanup(ctx);
        #ifdef FREERTOS
        ret_val = rt_unlock_mutex(&dcp_lock);
        if(kStatus_Success != ret_val) return ret_val;
        #endif
        return MCRYPTO_FAILURE;
    }

    #ifdef FREERTOS
    ret_val = rt_unlock_mutex(&dcp_lock);
    if(kStatus_Success != ret_val) return ret_val;
    #endif
    ctx->initialized = true;

    return MCRYPTO_OK;
}

mcrypto_status_t mcrypto_aes_ctx_cleanup(mcrypto_aes_ctx_t* ctx)
{
    if (NULL != ctx)
    {
        memset ( (void*) ctx, 0x00, sizeof(mcrypto_aes_ctx_t));
    }

    return MCRYPTO_OK;
}

bool mcrypto_aes_ctx_initialized(mcrypto_aes_ctx_t* ctx)
{
    if(NULL == ctx)
    {
        return false;
    }

    /* Compare to true to ensure that a random value ( != 1 ) in ctx->initialized does pass the test */
    return (true == ctx->initialized);
}

mcrypto_status_t mcrypto_aes_ecb_encrypt(mcrypto_aes_ctx_t* ctx, const uint8_t* plaintext, uint8_t* ciphertext, size_t size)
{
    if ((NULL == ctx) || (NULL == plaintext) || (NULL == ciphertext) || (0 == size))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    if (!mcrypto_aes_ctx_initialized(ctx))
    {
        return MCRYPTO_CTX_NOT_INITIALIZED;
    }

    #ifdef FREERTOS
    status_t ret_val = rt_lock_mutex(&dcp_lock);
    if(kStatus_Success != ret_val) return ret_val;
    #endif
    /* make sure all data are written to memory, before DCP will start using them */
    SCB_CleanDCache_by_Addr((uint32_t*)&(ctx->dcp_handle), sizeof(dcp_handle_t));
    SCB_CleanDCache_by_Addr((uint32_t*)plaintext, size);

    if (kStatus_Success != DCP_AES_EncryptEcb(DCP_DEVICE, &(ctx->dcp_handle), plaintext, ciphertext, size))
    {
        #ifdef FREERTOS
        ret_val = rt_unlock_mutex(&dcp_lock);
        if(kStatus_Success != ret_val) return ret_val;
        #endif
        return MCRYPTO_FAILURE;
    }
    /* make sure we are do not have any part of just written memory in the cache */
    SCB_InvalidateDCache_by_Addr((uint32_t *)ciphertext, size);
    #ifdef FREERTOS
    ret_val = rt_unlock_mutex(&dcp_lock);
    if(kStatus_Success != ret_val) return ret_val;
    #endif
    return MCRYPTO_OK;
}

mcrypto_status_t mcrypto_aes_ecb_decrypt(mcrypto_aes_ctx_t* ctx, const uint8_t* ciphertext, uint8_t* plaintext, size_t size)
{
    if ((NULL == ctx) || (NULL == ciphertext) || (NULL == plaintext) || (0 == size))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    if (!mcrypto_aes_ctx_initialized(ctx))
    {
        return MCRYPTO_CTX_NOT_INITIALIZED;
    }

    #ifdef FREERTOS
    status_t ret_val = rt_lock_mutex(&dcp_lock);
    if(kStatus_Success != ret_val) return ret_val;
    #endif
    /* make sure all data are written to memory, before DCP will start using them */
    SCB_CleanDCache_by_Addr((uint32_t*)&(ctx->dcp_handle), sizeof(dcp_handle_t));
    SCB_CleanDCache_by_Addr((uint32_t*)ciphertext, size);


    if (kStatus_Success != DCP_AES_DecryptEcb(DCP_DEVICE, &(ctx->dcp_handle), ciphertext, plaintext, size))
    {
        #ifdef FREERTOS
        ret_val = rt_unlock_mutex(&dcp_lock);
        if(kStatus_Success != ret_val) return ret_val;
        #endif
        return MCRYPTO_FAILURE;
    }

    /* make sure we are do not have any part of just written memory in the cache */
    SCB_InvalidateDCache_by_Addr((uint32_t *)plaintext, size);
    #ifdef FREERTOS
    ret_val = rt_unlock_mutex(&dcp_lock);
    if(kStatus_Success != ret_val) return ret_val;
    #endif

    return MCRYPTO_OK;
}

/* Platform specific functions */

mcrypto_status_t mcrypto_set_dcp_channel(mcrypto_aes_ctx_t* ctx, dcp_channel_t channel)
{
    if (NULL == ctx)
    {
        return MCRYPTO_INVALID_ARGS;
    }

    ctx->dcp_handle.channel = channel;

    return MCRYPTO_OK;
}

mcrypto_status_t mcrypto_get_dcp_channel(mcrypto_aes_ctx_t* ctx, dcp_channel_t* channel)
{
    if ((NULL == ctx) || (NULL == channel))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    if (!mcrypto_aes_ctx_initialized(ctx))
    {
        return MCRYPTO_CTX_NOT_INITIALIZED;
    }

    *channel = ctx->dcp_handle.channel;

    return MCRYPTO_OK;
}


