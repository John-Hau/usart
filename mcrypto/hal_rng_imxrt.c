/**
 * @file hal_rng_imxrt.c
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief RNG HAL implementation for i.MX RT
 * @date 2020-05-27
 *
 * This module implements Random Number Generator functions on i.MX RT using its TRNG periphery.
 *
 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */
#include <stddef.h>
#include "fsl_trng.h"
#include "mcrypto.h"
#include "hal_rng.h"

#define TRNG_DEVICE TRNG

mcrypto_status_t mcrypto_rng_init(void* params)
{
    /* This implementation does not use any initialization parameters */
    (void) params;

    trng_config_t trng_config;
    
    /* 
     * Use the same default configuration with Von Neumann sampling mode just as it is done
     * in the NXP' TRNG driver example from SDK and flashloader.
     */
    if (kStatus_Success != TRNG_GetDefaultConfig(&trng_config))
    {
        return MCRYPTO_FAILURE;
    }

    trng_config.sampleMode = kTRNG_SampleModeVonNeumann;

    if (kStatus_Success != TRNG_Init(TRNG_DEVICE, &trng_config))
    {    
        return MCRYPTO_FAILURE;
    }

    return MCRYPTO_OK;
}

mcrypto_status_t mcrypto_rng_deinit()
{
    TRNG_Deinit(TRNG_DEVICE);

    return MCRYPTO_OK;
}

mcrypto_status_t mcrypto_rng_get_data(void* data, uint32_t size)
{
    if((NULL == data) || (size == 0)) 
    {
        return MCRYPTO_INVALID_ARGS;
    }

    if (kStatus_Success != TRNG_GetRandomData(TRNG_DEVICE, data, size))
    {    
        return MCRYPTO_FAILURE;
    }

    return MCRYPTO_OK;
}