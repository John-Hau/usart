/**
 * @file kdf.h
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief KDF (Key Derivation Function) implementation
 * @date 2020-05-18
 *
 * This module implements Key Derivation Function in counter mode 
 * using AES-128 CMAC as pseudorandom function as defined in 
 * NIST Special Publication 800-108, October 2009.
 * 
 * This modules is hardware and key independent in the sense that it calls CMAC module
 * to perform all key-dependent computations.
 * 
 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */

#ifndef MCRYPTO_KDF_H
#define MCRYPTO_KDF_H

#include <stdint.h>
#include "mcrypto.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Compute a key derived from device protected key.
 * 
 * Function uses KDF in counter mode using AES-128 CMAC as defined 
 * by NIST Special Publication 800-108, October 2009.
 * 
 * @param[out] key Derived key
 * @param key_len required size of the derived key in bytes. Must be between 16 bytes and (inclusiding) 4096 bytes .
 * @param label Label used in key derivation, for example derived key usage. Must not be NULL.
 * @param label_len Label byte length. Must not be zero.
 * @param context Context used in key derivation, for example nonce, or device identity. Must not be NULL.
 * @param context_len Context byte length. Must not be zero.
 * @return mcrypto_status_t 
 */
mcrypto_status_t mcrypto_kdf_with_device_key(uint8_t* key, uint32_t key_len, const unsigned char* label, uint32_t label_len, const unsigned char* context, uint32_t context_len);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* MCRYPTO_KDF_H */