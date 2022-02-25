/**
 * @file aes-xts.c
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief Implementation of XTS-AES-128
 * @date 2020-05-29
 *
 * This module implements AES XTS as described by NIST Special Publication 800-38E
 * January, 2010 without the ciphertext stealing functionality - only complete
 * blocks can be encrypted / decrypted.
 *
 * All data and addresses must be 16-byte aligned and data sizes multiple of 16 bytes.
 *
 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */

#include <stdalign.h>
#include "aes-xts.h"

/**
 * @brief XTS data unit size.
 * Must be a power of 2 greater or equal to 16.
 */
const uint32_t mcrypto_xts_data_unit_size = 1024U;

/* GF(2^128) primitive element contant for 128-bit blocks */
#define RB_CONSTANT 0x87

/* Encryption / decryption function pointer type */
typedef mcrypto_status_t (*mcrypto_aes_ecb_crypt_func_t)(mcrypto_aes_ctx_t* ctx, const uint8_t* plaintext, uint8_t* ciphertext, size_t size);

/* Multiply by primitive element of GC(2^128) with little endian representation */
static void mcrypto_multiply_by_alpha_le(uint32_t* value);

static mcrypto_status_t mcrypto_aes_xts_internal(mcrypto_aes_ecb_crypt_func_t crypto_funct, mcrypto_aes_xts_ctx_t* ctx, uint64_t address,
                                            const uint8_t* input, uint8_t* output, uint32_t size);

mcrypto_status_t mcrypto_aes_xts_ctx_init(mcrypto_aes_xts_ctx_t* ctx, mcrypto_secret_key_t* key1, mcrypto_secret_key_t* key2)
{
    if ((NULL == ctx) || (NULL == key1) || (NULL == key2))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    mcrypto_status_t status;

    /* Initialize the AS XTS context with the provided 2 keys */

    status = mcrypto_aes_ctx_init_with_key(&(ctx->aes_ctx1), key1);

    if (MCRYPTO_OK != status)
    {
        mcrypto_aes_xts_ctx_cleanup(ctx);
        return status;
    }

    status = mcrypto_aes_ctx_init_with_key(&(ctx->aes_ctx2), key2);

    if (MCRYPTO_OK != status)
    {
        mcrypto_aes_xts_ctx_cleanup(ctx);
        return status;
    }

    return MCRYPTO_OK;
}

mcrypto_status_t mcrypto_aes_xts_ctx_cleanup(mcrypto_aes_xts_ctx_t* ctx)
{
    if (NULL != ctx)
    {
        memset ( (void*) ctx, 0x00, sizeof(mcrypto_aes_xts_ctx_t));
    }

    return MCRYPTO_OK;
}

mcrypto_status_t mcrypto_aes_xts_encrypt(mcrypto_aes_xts_ctx_t* ctx, uint64_t address,
                                            const uint8_t* plaintext, uint8_t* ciphertext, uint32_t size)
{
    return mcrypto_aes_xts_internal(mcrypto_aes_ecb_encrypt, ctx, address, plaintext, ciphertext, size);
}

mcrypto_status_t mcrypto_aes_xts_decrypt(mcrypto_aes_xts_ctx_t* ctx, uint64_t address,
                                            const uint8_t* ciphertext, uint8_t* plaintext, uint32_t size)
{
    return mcrypto_aes_xts_internal(mcrypto_aes_ecb_decrypt, ctx, address, ciphertext, plaintext, size);
}

static mcrypto_status_t mcrypto_aes_xts_internal(mcrypto_aes_ecb_crypt_func_t crypto_funct, mcrypto_aes_xts_ctx_t* ctx, uint64_t address,
                                            const uint8_t* input, uint8_t* output, uint32_t size)
{
    if ((NULL == ctx) || (NULL == input) || (NULL == output) || (0 == size))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    /* Address must be 16-byte aligned, size must be multiple of 16 */
    if ( (address & 0x0f) || (size & 0x0f) )
    {
        return MCRYPTO_INVALID_ARGS;
    }

    mcrypto_status_t status;
    uint32_t i;
    uint32_t input_i = 0;

    alignas(4) uint8_t tweak[MCRYPTO_AES_BLOCK_SIZE];
    alignas(4) uint8_t buffer[MCRYPTO_AES_BLOCK_SIZE];

    uint32_t aes_block_index; /* Index of the 16-byte block inside the data block */

    while (input_i < size)
    {
        /* Encrypt all the data that are inside a single data unit */

        memset(tweak, 0x00, sizeof(tweak));

        /* Split the input address to:
         *  - data unit index - index of the data unit that the address is within
         *  - index of the 16-byte block inside the data unit
         */

        /* mcrypto_xts_data_unit_size is a power of 2 */
        aes_block_index = ((uint32_t)(address) % mcrypto_xts_data_unit_size) / MCRYPTO_AES_BLOCK_SIZE;
        *(uint64_t*)(tweak) = address / mcrypto_xts_data_unit_size;

        /* Encrypt the tweak in place using the key2 */
        status = mcrypto_aes_ecb_encrypt(&(ctx->aes_ctx2), tweak, tweak, sizeof(tweak));
        if (MCRYPTO_OK != status)
        {
            return status;
        }

        /* Modify the tweak based on the aes_block_index */
        for (i = 0; i < aes_block_index; ++i)
        {
            mcrypto_multiply_by_alpha_le((uint32_t*)tweak);
        }

        /* Encrypt/Decrypt all blocks inside this data unit */
        while ((input_i < size) && (aes_block_index*MCRYPTO_AES_BLOCK_SIZE < mcrypto_xts_data_unit_size))
        {
            /* XOR the tweak with the input */
            for(i=0;i<MCRYPTO_AES_BLOCK_SIZE;i++)
            {
                buffer[i] = input[input_i+i] ^ tweak[i];
            }

            status = crypto_funct(&(ctx->aes_ctx1), buffer, buffer, sizeof(buffer));
            if (MCRYPTO_OK != status)
            {
                return status;
            }

            /* XOR the tweak after the encryption / decryption and write it to the result */
            for(i=0;i<MCRYPTO_AES_BLOCK_SIZE;i++)
            {
                buffer[i] = buffer[i] ^ tweak[i];
                output[input_i+i] = buffer[i];
            }

            /* Prepare the tweak for the next block */
            /* Unlike in XEX, we use also aes_block_index = 0 */
            mcrypto_multiply_by_alpha_le((uint32_t*)tweak);

            input_i += MCRYPTO_AES_BLOCK_SIZE;
            aes_block_index += 1;
        }

        /**
         * We have encrypted all the data that are inside one data unit.
         * Before continuing to the next data unit with the rest of the data (if any),
         * adjust the address to the start of the next data unit.
         */
        address = address - (address % mcrypto_xts_data_unit_size) + mcrypto_xts_data_unit_size;
    }

    return MCRYPTO_OK;
}

static void mcrypto_multiply_by_alpha_le(uint32_t* value)
{
    if(NULL == value)
    {
        return;
    }

    uint32_t i,tmp,carry;

    carry = 0;

    for (i = 0; i != MCRYPTO_AES_BLOCK_SIZE / sizeof(uint32_t); ++i)
    {
        tmp = value[i];
        value[i] = (value[i] << 1) | carry;
        carry = tmp >> 31;
    }

    /* If msb(input value) = 1, XOR RB constant */
    if (carry)
    {
        ((uint8_t*)value)[0] ^= RB_CONSTANT;
    }
}
