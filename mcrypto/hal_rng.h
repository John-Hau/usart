/**
 * @file hal_rng.h
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief Hardware Abstraction Layer for Random Number Generator.
 * @date 2020-05-27
 *
 * This API provides functions for random number generation (RNG).
 * There functions should be implemented for each hardware platform.

 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */

#ifndef MCRYPTO_HAL_RNG_H
#define MCRYPTO_HAL_RNG_H

#include <stdint.h>
#include "mcrypto.h"

/* 
 * Include platform dependent headers.
 */

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Initialize platform random number generator.
 *
 * @param params Platform dependent initialization parameters. 
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_rng_init(void* params);

/**
 * @brief Deinitialize platform random number generator.
 *
 * @return mcrypto_status_t
 */
mcrypto_status_t mcrypto_rng_deinit();

/**
 * @brief Get random data.
 * 
 * @param[out] data Generated random bytes 
 * @param size Number of bytes to be generated.
 * @return mcrypto_status_t 
 */
mcrypto_status_t mcrypto_rng_get_data(void* data, uint32_t size);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* MCRYPTO_HAL_RNG_H */