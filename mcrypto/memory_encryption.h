/**
 * @file memory_encryption.h
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
 * 
 * The size of the encrypted data must be a multiple of 16. 
 * (Target / source) data address must be 16-byte aligned.
 * 
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

#ifndef MEMORY_ENCRYPTION_H
#define MEMORY_ENCRYPTION_H

#include <stdint.h>
#include "aes-xts.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* Memory encryption context is the XTS-AES-128 context */
typedef mcrypto_aes_xts_ctx_t mcrypto_memenc_ctx_t;

/**
 * @brief Initialize memory encryption context.
 * 
 * Function initializes the context with a key derived from the device protected key
 * and the input label and context parameters.
 * 
 * @param[out] ctx Memory encryption context to be initialized.
 * @param label A string that identifies the purpose for the derived keying material. For example "SMB I/O module storage". Must not be NULL.
 * @param label_len Byte length of the label. Must be greater than 0. label_len + context_len <= 250
 * @param context Information further specifying the encryption context. This can be a dedicated storage nonce (if used) or 
 *                can be set to a fixed value if there is no such information. Must not be NULL.
 * @param context_len Byte length of the context. Must be greater than 0. label_len + context_len <= 250
 * @return mcrypto_status_t 
 */
mcrypto_status_t mcrypto_memenc_ctx_init(mcrypto_memenc_ctx_t* ctx, const unsigned char* label, uint32_t label_len, const unsigned char* context, uint32_t context_len);

/**
 * @brief Cleanup of the memory encryption context
 *
 * @param ctx Context to cleanup.
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_memenc_ctx_cleanup(mcrypto_memenc_ctx_t* ctx);

/**
 * @brief Function encrypts plaintext using context and address.
 *
 * Supports inplace encryption.
 *
 * @param ctx Initialized context.
 * @param address Target data address. Must be 16-byte aligned.
 * @param plaintext Plaintext to encrypt
 * @param[out] ciphertext Resulting ciphertext
 * @param size Size of the plaintext = size of the ciphertext. Must be a multiple of 16 bytes.
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_memenc_encrypt(mcrypto_memenc_ctx_t* ctx, uint32_t address, const uint8_t* plaintext, uint8_t* ciphertext, size_t size);

/**
 * @brief Decrypts the input ciphertext using input context and address.
 *
 * Supports inplace decryption.
 *
 * @param ctx Initialized context.
 * @param address Address where the data were read from. Must be 16-byte aligned.
 * @param ciphertext Ciphertext to decrypt
 * @param[out] plaintext Decrypted plaintext
 * @param size Size of the plaintext = size of the ciphertext. Must be a multiple of 16 bytes.
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_memenc_decrypt(mcrypto_memenc_ctx_t* ctx, uint32_t address, const uint8_t* ciphertext, uint8_t* plaintext, size_t size);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* MEMORY_ENCRYPTION_H */