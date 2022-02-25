/**
 * @file hal_aes.h
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief Hardware Abstraction Layer for AES-128
 * @date 2020-05-18
 *
 * This API provides function for AES-128 encryption used by MCRYPTO library.
 * There functions should be implemented for each hardware platform.

 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */

#ifndef MCRYPTO_HAL_AES_H
#define MCRYPTO_HAL_AES_H

#include <stdint.h>
#include "mcrypto.h"

/* 
 * Include platform dependent headers.
 * These must define the aes context type mcrypto_aes_ctx_t 
 * and optionally other platform dependent functions.
 */
#include "hal_aes_imxrt.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Initialize platform dependent AES context with in the input secret key.
 *
 * @param ctx AES context
 * @param key The secret key. Key data must be 32-bit aligned.
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_aes_ctx_init_with_key(mcrypto_aes_ctx_t* ctx, const mcrypto_secret_key_t* key);

/**
 * @brief Initialize platform dependent AES context with the device unique secret key.
 *
 * The device unique key is not know to the caller.
 *
 * @param[out] ctx AES context
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_aes_ctx_init_with_devkey(mcrypto_aes_ctx_t* ctx);

/**
 * @brief Cleanup AES context
 *
 * @param ctx AES context
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_aes_ctx_cleanup(mcrypto_aes_ctx_t* ctx);

/**
 * @brief Returns true is the context is initialized.
 * 
 * @param ctx AES context
 * @return true Context is initialized.
 * @return false Otherwise
 */
bool mcrypto_aes_ctx_initialized(mcrypto_aes_ctx_t* ctx);

/**
 * @brief Encrypt one or multiple blocks with AES in ECB mode.
 *
 * The plaintext and ciphertext can overlap in memory.
 *
 * @param ctx AES context. Device specific.
 * @param plaintext Input plaintext to encrypt
 * @param[out] ciphertext Output ciphertext
 * @param size Size of plaintext and ciphertext in bytes. Must be multiple of 16.
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_aes_ecb_encrypt(mcrypto_aes_ctx_t* ctx, const uint8_t* plaintext, uint8_t* ciphertext, size_t size);

/**
 * @brief Decrypt one or multiple blocks with AES in ECB mode.
 *
 * The plaintext and ciphertext can overlap in memory.
 *
 * @param ctx AES context. Device specific.
 * @param ciphertext Input ciphertext to decrypt
 * @param[out] plaintext Output plaintext
 * @param size Size of plaintext and ciphertext in bytes. Must be multiple of 16.
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_aes_ecb_decrypt(mcrypto_aes_ctx_t* ctx, const uint8_t* ciphertext, uint8_t* plaintext, size_t size);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* MCRYPTO_HAL_AES_H */