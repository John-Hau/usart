/**
 * @file data_authentication.c
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief Data authentication
 * @date 2020-05-26
 *
 * This module provides function for data authentication.
 * The key used for authentication is derived from the device unique key.
 * The intended usage is for authentication of data stored in external memories.
 * See the documentation for more information. 
 * 
 * The authentication tag is computed by AES-128 CMAC 
 * as defined in NIST Special Publication 800-38B, May 2005.
 * 
 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */

#include <assert.h>
#include "kdf.h"
#include "cmac1.h"
#include "data_authentication.h"

mcrypto_status_t mcrypto_dauth_ctx_init(mcrypto_dauth_ctx_t* ctx)
{
    if (NULL == ctx)
    {
        return MCRYPTO_INVALID_ARGS;
    }

    mcrypto_status_t status;
    mcrypto_secret_key_t key;

    /* Key generation inputs */
    unsigned char label[] = "MCrypto Data Authentication";
    unsigned char context[] = { 0xCA, 0xFE, 0xBA, 0xBE }; /* There is no context (in the NIST SP800 sense), so use fixed magic value */
    
    /* Generate the secret key for data authentication from the device protected key */
    status = mcrypto_kdf_with_device_key((uint8_t*)(key.data), sizeof(key.data), label, sizeof(label), context, sizeof(context));

    if (MCRYPTO_OK != status)
    {
        return status;
    }

    /* Initialize the context with the derived key */
    status = mcrypto_aes_ctx_init_with_key((mcrypto_aes_ctx_t*)ctx, &key);

    memset(&key, 0x00, sizeof(mcrypto_secret_key_t));

    if (MCRYPTO_OK != status)
    {
        return status;
    }

    return MCRYPTO_OK;
}

mcrypto_status_t mcrypto_dauth_ctx_cleanup(mcrypto_dauth_ctx_t* ctx)
{
    return mcrypto_aes_ctx_cleanup((mcrypto_aes_ctx_t*)ctx);
}

mcrypto_status_t mcrypto_dauth_compute_tag(mcrypto_dauth_ctx_t* ctx, const void* data, uint32_t size, mcrypto_auth_tag_t* auth_tag)
{
    if ((NULL == ctx) || ((NULL == data) && (size > 0)) || (NULL == auth_tag))
    {
        return MCRYPTO_INVALID_ARGS;
    }
    
    return mcrypto_aes_cmac((mcrypto_aes_ctx_t*)ctx, data, size, (mcrypto_auth_tag_t *) auth_tag);
}

mcrypto_status_t mcrypto_dauth_verify_tag(mcrypto_dauth_ctx_t* ctx, const void* data, uint32_t size, const mcrypto_auth_tag_t* auth_tag)
{
    if ((NULL == ctx) || ((NULL == data) && (size > 0)) || (NULL == auth_tag))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    mcrypto_status_t status;
    mcrypto_auth_tag_t computed_tag;

    status = mcrypto_dauth_compute_tag(ctx, data, size, &computed_tag);

    if (MCRYPTO_OK != status)
    {
        return status;
    }

    if (0 != memcmp(computed_tag.data, auth_tag->data, sizeof(computed_tag.data)))
    {
        return MCRYPTO_INVALID_TAG;
    }

    return MCRYPTO_OK;
}

mcrypto_status_t mcrypto_dauth_compute_tag_easy(const void* data, uint32_t size, mcrypto_auth_tag_t* auth_tag)
{
    if (((NULL == data) && (size > 0)) || (NULL == auth_tag))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    mcrypto_status_t status;
    mcrypto_dauth_ctx_t ctx;

    status = mcrypto_dauth_ctx_init(&ctx);

    if(MCRYPTO_OK != status)
    {
        goto exit;
    }

    status = mcrypto_dauth_compute_tag(&ctx, data, size, auth_tag);

    exit:

    mcrypto_dauth_ctx_cleanup(&ctx);
    return status;
}

mcrypto_status_t mcrypto_dauth_verify_tag_easy(const void* data, uint32_t size, const mcrypto_auth_tag_t* auth_tag)
{
    if (((NULL == data) && (size > 0)) || (NULL == auth_tag))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    mcrypto_status_t status;
    mcrypto_dauth_ctx_t ctx;

    status = mcrypto_dauth_ctx_init(&ctx);

    if(MCRYPTO_OK != status)
    {
        goto exit;
    }

    status = mcrypto_dauth_verify_tag(&ctx, data, size, auth_tag);

    exit:

    mcrypto_dauth_ctx_cleanup(&ctx);
    return status;
}
