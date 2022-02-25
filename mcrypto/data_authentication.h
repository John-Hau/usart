/**
 * @file data_authentication.h
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief Data authentication API
 * @date 2020-05-12
 *
 * This is MCrypto external API - it is not used internally by MCrypto.
 * 
 * This API provides function for data authentication.
 * The key used for authentication is derived from the device unique key.
 * The intended usage is for authentication of data stored in external memories.
 * See the documentation for more information. 
 * 
 * The authentication tag is computed by AES-128 CMAC 
 * as defined in NIST Special Publication 800-38B, May 2005.
 * 
 * 
 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */

#ifndef DATA_AUTHENTICATION_H
#define DATA_AUTHENTICATION_H

#include <stdint.h>
#include "mcrypto.h"
#include "hal_aes.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* Data authentication context is an AES context as we use AES-128 CMAC */
typedef mcrypto_aes_ctx_t mcrypto_dauth_ctx_t;

/**
 * @brief Initialize data authentication context that can be used to compute data authentication tag.
 * 
 * @param[out] ctx Data authentication context
 * @return mcrypto_status_t 
 */
mcrypto_status_t mcrypto_dauth_ctx_init(mcrypto_dauth_ctx_t* ctx);

/**
 * @brief Clean up the data authentication context.
 * 
 * @param ctx Context to be cleaned. 
 * @return mcrypto_status_t 
 */
mcrypto_status_t mcrypto_dauth_ctx_cleanup(mcrypto_dauth_ctx_t* ctx);

/**
 * @brief Compute authentication tag of the input data
 * 
 * @param ctx Authentication context to use. The context must be already initialized.
 * @param data Data to be authenticated
 * @param size Size of the data to be authenticated
 * @param auth_tag[out] Computed authentication tag
 * @return mcrypto_status_t 
 */
mcrypto_status_t mcrypto_dauth_compute_tag(mcrypto_dauth_ctx_t* ctx, const void* data, uint32_t size, mcrypto_auth_tag_t* auth_tag);

/**
 * @brief Verify authetication tag of the input data
 * 
 * Function computes authentication tag and compares it to the input tag.
 * 
 * @param ctx Authentication context to use. The context must be already initialized.
 * @param data Data that the tag authenticates
 * @param size Size of the data
 * @param auth_tag Authentication tag for verification
 * @return mcrypto_status_t If the tag if invalid, function returns MCRYPTO_INVALID_TAG
 */
mcrypto_status_t mcrypto_dauth_verify_tag(mcrypto_dauth_ctx_t* ctx, const void* data, uint32_t size, const mcrypto_auth_tag_t* auth_tag);


/** Slow version without contex */

/**
 * @brief Compute authentication tag of the input data without context.
 * 
 * Function computes authentication tag without previous context initialization at the cost of slower execution.
 * 
 * @param data Data to be authenticated
 * @param size Size of the data to be authenticated
 * @param auth_tag[out] Computed authentication tag
 * @return mcrypto_status_t 
 */
mcrypto_status_t mcrypto_dauth_compute_tag_easy(const void* data, uint32_t size, mcrypto_auth_tag_t* auth_tag);

/**
 * @brief Verify authetication tag of the input data
 * 
 * Function computes authentication tag and compares it to the input tag without previous context initialization at the cost of slower execution.
 * 
 * @param data Data that the tag authenticates
 * @param size Size of the data
 * @param auth_tag Authentication tag for verification
 * @return mcrypto_status_t If the tag if invalid, function returns MCRYPTO_INVALID_TAG
 */
mcrypto_status_t mcrypto_dauth_verify_tag_easy(const void* data, uint32_t size, const mcrypto_auth_tag_t* auth_tag);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* DATA_AUTHENTICATION_H */