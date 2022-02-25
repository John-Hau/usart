/**
 * @file memory_encryption.c
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief Memory encryption API
 * @date 2020-06-12
 *
 * This is MCrypto external API - it is not used internally by MCrypto.
 * 
 * This API provides functions for transparent memory encryption using 
 * a key derived from the device unique key.
 *
 * The intended usage is for data encryption for external memories. 
 * See SEST Secure Storage documentation for more information. 
 * 
 * The encryption is done by XTS-AES-128 (defined in NIST Special Publication 800-38E
 * January, 2010) without the ciphertext stealing functionality - only complete
 * blocks can be encrypted / decrypted.
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
#include "aes-xts.h"
#include "memory_encryption.h"

mcrypto_status_t mcrypto_memenc_ctx_init(mcrypto_memenc_ctx_t* ctx, const unsigned char* label, uint32_t label_len, const unsigned char* context, uint32_t context_len)
{
    if ((NULL == ctx) || (NULL == label) || (0 == label_len) || (NULL == context) || (0 == context_len))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    mcrypto_status_t status;
    uint8_t keys[sizeof(mcrypto_secret_key_t)*2];

    /* Derive AES XTS keys using the KDF with device protected key and provided inputs */
    status = mcrypto_kdf_with_device_key(keys, sizeof(keys), label, label_len, context, context_len);

    if (MCRYPTO_OK != status)
    {
        goto exit;
    }

    /* Initialize the context with the generated keys */
    status = mcrypto_aes_xts_ctx_init((mcrypto_aes_xts_ctx_t*)(ctx), (mcrypto_secret_key_t *)keys, (mcrypto_secret_key_t *)(keys + sizeof(mcrypto_secret_key_t)));

    exit:
    memset(keys, 0x00, sizeof(keys));

    return status;
}

mcrypto_status_t mcrypto_memenc_ctx_cleanup(mcrypto_memenc_ctx_t* ctx)
{
    return mcrypto_aes_xts_ctx_cleanup((mcrypto_aes_xts_ctx_t*)(ctx));
}

mcrypto_status_t mcrypto_memenc_encrypt(mcrypto_memenc_ctx_t* ctx, uint32_t address, const uint8_t* plaintext, uint8_t* ciphertext, size_t size)
{
    if ((NULL == ctx) || (NULL == plaintext) || (NULL == ciphertext) || (0 == size))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    return mcrypto_aes_xts_encrypt((mcrypto_aes_xts_ctx_t*)(ctx), address, plaintext, ciphertext, size);
}

mcrypto_status_t mcrypto_memenc_decrypt(mcrypto_memenc_ctx_t* ctx, uint32_t address, const uint8_t* ciphertext, uint8_t* plaintext, size_t size)
{
    if ((NULL == ctx) || (NULL == ciphertext) || (NULL == plaintext))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    return mcrypto_aes_xts_decrypt((mcrypto_aes_xts_ctx_t*)(ctx), address, ciphertext, plaintext, size);
}
