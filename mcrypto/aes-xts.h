/**
 * @file aes-xts.h
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

#ifndef MCRYPTO_XTS_AES_H
#define MCRYPTO_XTS_AES_H

#include <stdint.h>
#include "hal_aes.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Platform dependent AES XTS context
 *
 * XTS-AES uses 2 keys so we keep 2 AES contexts
 *
 */
typedef struct tag_mcrypto_aes_xts_ctx
{
    mcrypto_aes_ctx_t aes_ctx1;
    mcrypto_aes_ctx_t aes_ctx2;
} mcrypto_aes_xts_ctx_t;

/**
 * @brief Initialize AES XTS context using 2 AES keys
 *
 * @param dcp_channel DCP channel to use for the subsequent AES computations.
 * @param ctx Context that will hold the AES DCP context including secret key.
 * @return mcrypto_status_t
 */

mcrypto_status_t mcrypto_aes_xts_ctx_init(mcrypto_aes_xts_ctx_t * ctx, mcrypto_secret_key_t* key1, mcrypto_secret_key_t* key2);

/**
 * @brief Cleanup of the AES XTS context
 *
 * @param ctx
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_aes_xts_ctx_cleanup(mcrypto_aes_xts_ctx_t* ctx);

/**
 * @brief Encrypt data with AES in XTS mode.
 *
 * Both plaintext and ciphertext must be 16 bytes aligned.
 * Size of the plaintext (and hence also ciphertext) must be divisible by 16.
 *
 * @param ctx Initialized AES XTS context
 * @param address Target address where the data will be stored. Must be 16-bytes aligned! Used as a initial tweak, which can be 64-bit long.
 * @param plaintext Plaintext for encryption. Must be 16-bytes aligned!
 * @param[out] ciphertext Resulting ciphertext. Can be the same as plaintext, but the plaintext block must not only partially overlap with ciphertext.
 * @param size Size of the plaintext in bytes. Must be divisible by 16!
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_aes_xts_encrypt(mcrypto_aes_xts_ctx_t* ctx, uint64_t address,
                                            const uint8_t* plaintext, uint8_t* ciphertext, uint32_t size);

/**
 * @brief Decrypt data with AES in XTS mode.
 *
 * Both plaintext and ciphertext must be 16 bytes aligned.
 * Size of the ciphertext (and hence also plaintext) must be divisible by 16.
 *
 * @param ctx Initialized AES XTS context
 * @param address Address from which the data were read. Mut be 16-bytes aligned! Used as a initial tweak, which can be 64-bit long.
 * @param ciphertext Ciphertext to decrypt. Must be 16-bytes aligned!
 * @param[out] plaintext Resulting plaintext.  Can be the same as ciphertext, but the plaintext block must not only partially overlap with ciphertext.
 * @param size Size of the ciphertext in bytes. Must be divisible by 16!
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_aes_xts_decrypt(mcrypto_aes_xts_ctx_t* ctx, uint64_t address,
                                            const uint8_t* ciphertext, uint8_t* plaintext, uint32_t size);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* MCRYPTO_XTS_AES_H */