/** \file
 *  \brief  Gen random data host interface for VSOM Platform
 *
 *  \date   November 2019
 *
 *  \copyright Honeywell
 *  ALL RIGHTS RESERVED, Honeywell Confidential and Proprietary.
 */

#include "rng_host.h"


#if defined(FSL_FEATURE_SOC_TRNG_COUNT) && (FSL_FEATURE_SOC_TRNG_COUNT > 0)
#if defined(TRNG)
#define TRNG0 TRNG
#endif
#endif


status_t rt_rng_vector_generate(uint8_t * const p_target, size_t size)
{
    return TRNG_GetRandomData(TRNG0, p_target, size);   
}


status_t rt_rng_test(void)
{
    status_t ret_val;
    uint8_t rng_buff[32];
    
    memset(rng_buff, 0, sizeof(rng_buff));
    ret_val = rt_rng_vector_generate(rng_buff, sizeof(rng_buff));
    if(kStatus_Success != ret_val)
    {
        PRINTF("RNG test fail!");
        return ret_val;
    }

    for(uint8_t i=0;i<32;i++)
    {
        PRINTF("0x%02x ", rng_buff[i]);
    }
    PRINTF("\r\n");
    
    return kStatus_Success;
}


