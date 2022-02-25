/**
 * @file cmac.h
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief CMAC (Cipher-based Message Authentication Code) implementation as per NIST SP800-38B
 * @date 2020-05-18
 *
 * This API provides function for CMAC computation with either user provided key
 * or the device protected secret key.
 *
 * This module implements CMAC as defined in NIST Special Publication 800-38B, May 2005.
 * It uses platform independent AES-128 calls defined in hal_aes.h
 *
 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */

#ifndef MCRYPTO_CMAC_H
#define MCRYPTO_CMAC_H

#include <stdint.h>
#include "hal_aes.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Computes 16-byte AES-128 CMAC using device protected secret key.
 * 
 * Easy to use function - it does not require any (previously initialized) context.
 *
 * @param data Pointer to the input data to the tag computation. Can be NULL for empty message, size must be 0.
 * @param size Number of bytes of the message. Can be 0 for empty message.
 * @param[out] auth_tag Computed authentication tag.
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_aes_cmac_with_devkey(const void* data, uint32_t size, mcrypto_auth_tag_t* auth_tag);

/**
 * @brief Computes 16-byte AES-128 CMAC using input secret key.
 * 
 * Easy to use function - it does not require any (previously initialized) context.
 *
 * @param key Secret key
 * @param data Pointer to the input data to the tag computation. Can be NULL for empty message, size must be 0.
 * @param size Number of bytes of the message. Can be 0 for empty message.
 * @param[out] auth_tag Computed authentication tag.
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_aes_cmac_with_key(const mcrypto_secret_key_t* key, const void* data, uint32_t size, mcrypto_auth_tag_t* auth_tag);

/**
 * @brief Computes 16-byte AES-128 CMAC using input AES context.
 * 
 * Function computes CMAC using previously initialized AES context. Use functions from hal_aes.h for context initialization.
 * 
 * @param ctx Initialized AES context
 * @param data Pointer to the input data to the tag computation. Can be NULL for empty message, size must be 0.
 * @param size Number of bytes of the message. Can be 0 for empty message.
 * @param[out] auth_tag Computed authentication tag.
 * @return mcrypto_status_t 
 */
mcrypto_status_t mcrypto_aes_cmac(mcrypto_aes_ctx_t* ctx, const void* data, uint32_t size, mcrypto_auth_tag_t* auth_tag);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* MCRYPTO_CMAC_H */