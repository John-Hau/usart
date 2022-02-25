/**
 * @file hal_aes_imxrt.h
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief AES platform specific functions on i.MX RT
 * @date 2020-05-18
 *
 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */


#ifndef MCRYPTO_HAL_AES_IMXRT_H
#define MCRYPTO_HAL_AES_IMXRT_H

#include "fsl_dcp.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


/**
 * @brief Platform dependent AES context
 */
typedef struct tag_mcrypto_aes_ctx {

    /** @brief i.MX RT DCP handle */
    dcp_handle_t dcp_handle;

    /** @brief Trues if the context is initialized */
    bool initialized;
} mcrypto_aes_ctx_t;

/**
 * @brief Set the dcp channel that is be used by the context
 * 
 * @param ctx AES context
 * @param channel DCP channel
 * @return mcrypto_status_t 
 */
mcrypto_status_t mcrypto_set_dcp_channel(mcrypto_aes_ctx_t* ctx, dcp_channel_t channel);

/**
 * @brief Get the dcp channel that is be used by the context
 * 
 * @param ctx AES context
 * @param[out] channel DCP channel used by the context
 * @return dcp_channel_t 
 */
mcrypto_status_t mcrypto_get_dcp_channel(mcrypto_aes_ctx_t* ctx, dcp_channel_t* channel);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* MCRYPTO_HAL_AES_IMXRT_H */