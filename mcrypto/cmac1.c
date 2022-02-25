/**
 * @file cmac.c
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief CMAC (Cipher-based Message Authentication Code) implementation as per NIST SP800-38B
 * @date 2020-05-18
 *
 * This API provides function for CMAC computation with either user provided key
 * or the device protected secret key.
 *
 * This module implements CMAC as defined in NIST Special Publication 800-38B, May 2005.
 * It uses platform independent AES calls defined in hal_aes.h
 *
 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */

#include <stdalign.h>
#include "hal_aes.h"
#include "cmac1.h"

/* GF(2^128) primitive element contant for 128-bit blocks */
#define RB_CONSTANT 0x87

/* Local functions */
static void mcrypto_multiply_by_alpha(uint8_t* value);

mcrypto_status_t mcrypto_aes_cmac_with_devkey(const void* data, uint32_t size, mcrypto_auth_tag_t* auth_tag)
{
    if((NULL == auth_tag) || ((NULL == data) && (size > 0)))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    mcrypto_status_t status;
    mcrypto_aes_ctx_t ctx;

    status = mcrypto_aes_ctx_init_with_devkey(&ctx);

    if (MCRYPTO_OK != status)
    {
        goto exit;
    }

    status = mcrypto_aes_cmac(&ctx, data, size, auth_tag);

    exit:

    mcrypto_aes_ctx_cleanup(&ctx);
    return status;
}

mcrypto_status_t mcrypto_aes_cmac_with_key(const mcrypto_secret_key_t* key, const void* data, uint32_t size, mcrypto_auth_tag_t* auth_tag)
{
    if( (NULL == key) || (NULL == auth_tag) || ((NULL == data) && (size > 0)))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    mcrypto_status_t status;
    mcrypto_aes_ctx_t ctx;

    status = mcrypto_aes_ctx_init_with_key(&ctx, key);

    if (MCRYPTO_OK != status)
    {
        goto exit;
    }

    status = mcrypto_aes_cmac(&ctx, data, size, auth_tag);

    exit:

    mcrypto_aes_ctx_cleanup(&ctx);
    return status;
}

mcrypto_status_t mcrypto_aes_cmac(mcrypto_aes_ctx_t* ctx, const void* data, uint32_t size, mcrypto_auth_tag_t* auth_tag)
{
    if ((NULL == ctx) || (NULL == auth_tag) || ((NULL == data) && (size > 0)))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    if (!mcrypto_aes_ctx_initialized(ctx))
    {
        return MCRYPTO_CTX_NOT_INITIALIZED;
    }

    mcrypto_status_t status;
    uint32_t i;

    alignas(4) uint8_t buffer[MCRYPTO_AES_BLOCK_SIZE] = {0U};

    alignas(4) uint8_t subkey[MCRYPTO_AES_BLOCK_SIZE] = {0U};

    uint8_t* message = (uint8_t*) data;

    /* --------------- Process all message blocks expect the last one --------------- */

    while (size > MCRYPTO_AES_BLOCK_SIZE)
    {
        /* XOR the message block to the context */
        for (i=0;i<MCRYPTO_AES_BLOCK_SIZE;++i)
        {
            buffer[i] ^= *message++;
        }

        /* Encrypt with AES in ECB mode */
        status = mcrypto_aes_ecb_encrypt(ctx, buffer, buffer, sizeof(buffer));

        if(MCRYPTO_OK != status)
        {
            return status;
        }

        size -= MCRYPTO_AES_BLOCK_SIZE;
    }

    /* --------------- Generate subkey1 --------------- */

    status = mcrypto_aes_ecb_encrypt(ctx, subkey, subkey, sizeof(subkey));
    if(MCRYPTO_OK != status)
    {
        return status;
    }

    mcrypto_multiply_by_alpha(subkey);


    /* --------------- Handle the last block --------------- */

    /* XOR the remaining data */
    for (i=0;i<size;++i)
    {
        buffer[i] ^= *message++;
    }

    /* If the last block is incomplete, add padding and convert K1 into K2 */
    if (size < MCRYPTO_AES_BLOCK_SIZE)
    {
        buffer[size] ^= 0x80;

        mcrypto_multiply_by_alpha(subkey);
    }

    /* XOR the subkey (K1 or K2) with the last block */
    for (i=0;i<MCRYPTO_AES_BLOCK_SIZE; ++i)
    {
        buffer[i] ^= subkey[i];
    }

    /* Encrypt with AES in ECB mode */
    status = mcrypto_aes_ecb_encrypt(ctx, buffer, buffer, sizeof(buffer));
    if(MCRYPTO_OK != status)
    {
        return status;
    }

    /* Copy the result */
    memcpy((void*) auth_tag->data, (void*) buffer, sizeof(buffer));

    return MCRYPTO_OK;
}

static void mcrypto_multiply_by_alpha(uint8_t* value)
{
    if(NULL == value)
    {
        return;
    }

    int i,tmp,carry;

    carry = 0;

    for (i=(MCRYPTO_AES_BLOCK_SIZE-1);i>=0;--i)
    {
        tmp = value[i];
        tmp <<=1;

        if (carry)
        {
            tmp |= 0x01;
        }
        value[i] = (uint8_t) tmp;
        carry = tmp & 0x100;
    }

    /* If msb(input value) = 1, XOR RB constant */
    if (carry)
    {
        value[MCRYPTO_AES_BLOCK_SIZE-1] ^= RB_CONSTANT;
    }
}

