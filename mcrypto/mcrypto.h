/**
 * @file mcrypto.h
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief Mini crypto common header
 * @date 2020-05-18
 *
 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */

#ifndef MCRYPTO_H
#define MCRYPTO_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/**
 * @brief AES-128 block size in bytes
 */
#define MCRYPTO_AES_BLOCK_SIZE (16U)

/**
 * @brief Number of words of the secret key.
 *
 * This module uses 16 byte secret keys.
 */
#define MCRYPTO_SECRET_KEY_WORDSIZE (4U)

/**
 * @brief Status code type.
 */
typedef uint32_t mcrypto_status_t;

/**
 * @brief Common status codes.
 */
enum mcrypto_common_status
{
    MCRYPTO_OK                       = 0,    ///< Operation completed successfully.
    MCRYPTO_FAILURE                  = 1,    ///< Operation failed, unspecified error.
    MCRYPTO_INVALID_ARGS             = 2,    ///< Invalid arguments passed to a function.
    MCRYPTO_CTX_NOT_INITIALIZED      = 3,    ///< Context is not initialized
    MCRYPTO_INVALID_TAG              = 4,    ///< Invalid authentication tag
};

/**
 * @brief 16-byte secret key.
 */
typedef struct tag_mcrypto_secret_key
{
    /** @brief secret key data */
    uint32_t data[MCRYPTO_SECRET_KEY_WORDSIZE];
} mcrypto_secret_key_t;

/**
 * @brief Number of words of the 16-byte authentication tag
 * 
 * MCrypto computes 16-byte authentication tags.
 *
 */
#define MCRYPTO_AUTH_TAG_WORDSIZE (4U)
#define MCRYPTO_AUTH_TAG_SIZE (16U)

/**
 * @brief Authentication tag.
 */
typedef struct tag_mcrypto_auth_tag
{
    /** @brief authentication tag data */
    uint32_t data[MCRYPTO_AUTH_TAG_WORDSIZE];
} mcrypto_auth_tag_t;

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* MCRYPTO_H */