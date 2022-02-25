/** \file
 *  \brief  Gen random data host interface for VSOM Platform
 *
 *  \date   November 2019
 *
 *  \copyright Honeywell
 *  ALL RIGHTS RESERVED, Honeywell Confidential and Proprietary.
 */


#ifndef __RNG_HOST_H__
#define __RNG_HOST_H__

#include "fsl_debug_console.h"
#include "fsl_common.h"
#include "fsl_trng.h"
   
status_t rt_rng_vector_generate(uint8_t * const p_target, size_t size);

status_t rt_rng_test(void);

#endif
