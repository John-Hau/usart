#ifndef __CHKSUM_HOST_H__
#define __CHKSUM_HOST_H__

#include "fsl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t rt_checksum_crc(uint8_t const * p_data, uint32_t size);

bool rt_checksum_crc_ok(uint8_t const * p_data, uint32_t size, uint32_t crc_chk);


#ifdef __cplusplus
}
#endif

#endif

