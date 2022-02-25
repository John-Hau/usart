#ifndef __ATECC_HOST_H__
#define __ATECC_HOST_H__

#include "fsl_common.h"

#ifdef  CONFIG_RT_ECCHIP_SUPPORT
#include "atecc608_commands.h"
#include "prov/atecc608_conf.h"
#include "prov/provision.h"
#include "tuning/tuning.h"
#include "app/secure_boot/crypto_device_app.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
  
  
#define kStatusGroup_ATECC        400

enum _atecc_status
{  
    kStatusECCInitError = MAKE_STATUS(kStatusGroup_ATECC, 0),
    kStatusECCSelftestErr = MAKE_STATUS(kStatusGroup_ATECC, 1),
    kStatusECCCHKMAC = MAKE_STATUS(kStatusGroup_ATECC, 2),
    kStatusECCSETLATCH = MAKE_STATUS(kStatusGroup_ATECC, 3),
    kStatusECCGETLATCH = MAKE_STATUS(kStatusGroup_ATECC, 4),
    kStatusECCCHKLATCH = MAKE_STATUS(kStatusGroup_ATECC, 5),
    kStatusECCCMDError = MAKE_STATUS(kStatusGroup_ATECC, 6),
    kStatusECCELSEError = MAKE_STATUS(kStatusGroup_ATECC, 7),
    kStatusECCKEYError = MAKE_STATUS(kStatusGroup_ATECC, 8),
};

#define RT_ATECC_VER_OFFSET     0
#define RT_ATECC_VER_SIZE       28
#define RT_ATECC_CRC_SIZE       4

#define RT_CONF_KICKER_OFFSET   0
#define RT_CONF_APP_OFFSET      4
#define RT_CONF_OTA_OFFSET      8

#define CONF_SLOT               8

typedef enum
{
    verKicker = 0,
    verApp,
    verOTA,
    verAll,
} rt_ver_type;

status_t rt_get_read_key(uint8_t* read_key);

status_t rt_get_write_key(uint8_t* write_key);

status_t rt_get_latch_key(uint8_t* volatile_key);

status_t rt_unlock_keys(void);

status_t rt_atecc_config_locked(bool* lock);

status_t rt_atecc_encrypt_read(uint8_t* data_buff,
                               uint16_t slot,
                               size_t offset,
                               size_t length);

status_t rt_atecc_encrypt_write(uint8_t* data_buff,
                                uint16_t slot,
                                size_t offset,
                                size_t length);

status_t rt_atecc_init_version(void);

status_t rt_atecc_get_version(uint8_t * ver_data, rt_ver_type type);

bool rt_atecc_check_version(uint8_t * ver_data, rt_ver_type type);

status_t rt_atecc_set_version(uint8_t * ver_data, rt_ver_type type);

#ifdef __cplusplus
}
#endif

#endif

