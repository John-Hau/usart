/** \file
 *  \brief  ATECC host interface for VSOM Platform
 *
 *  \date   November 2019
 *
 *  \copyright Honeywell
 *  ALL RIGHTS RESERVED, Honeywell Confidential and Proprietary.
 */

#ifdef CONFIG_RT_ECCHIP_SUPPORT
#include "atecc_host.h"
#include "checksum_host.h"

#include "mem_map.h"
#include "rt_nvm.h"
#ifdef USE_SEST
#include "secure_storage/sest_operate.h"
extern secure_storage_t g_storage;
extern mcrypto_dauth_ctx_t ctx;
#endif

status_t rt_atecc_config_locked(bool* lock)
{
    ATCA_STATUS ret_code;
    uint8_t config_zone_read[ATCA_CONFIG_SIZE];
    
    ret_code = atecc608_read_config_zone_bytes(config_zone_read);
    if(ATCA_SUCCESS != ret_code)
    {
        return kStatusECCCMDError;
    }
    
    if (0x55 == config_zone_read[87])
    {
        *lock = false;
    }else if(0x00 == config_zone_read[87]){
        *lock = true;
    }else{
        return kStatusECCCMDError;
    }

    return kStatus_Success;
}


status_t rt_get_read_key(uint8_t* read_key)
{
    status_t ret_val;

#ifdef USE_SEST
    rt_read_key_info_t g_read_key_info;

    ret_val = rt_sest_read_data_mac(&ctx, &g_storage, RT_KEY_CERT_ADDRESS + RT_READ_KEY_OFFSET, sizeof(rt_read_key_info_t), g_read_key_info.read_key);
    if(kStatus_Success != ret_val) return ret_val;
    
    memcpy(read_key, g_read_key_info.read_key, RT_READ_KEY_SIZE);
#else
    ret_val = rt_nvm_read(RT_KEY_CERT_ADDRESS+RT_READ_KEY_OFFSET, 
                          read_key,
                          RT_READ_KEY_SIZE, 
                          NULL);
#endif
    return ret_val;
}


status_t rt_get_write_key(uint8_t* write_key)
{
    status_t ret_val;
#ifdef USE_SEST
    rt_write_key_info_t g_write_key_info;

    ret_val = rt_sest_read_data_mac(&ctx, &g_storage, RT_KEY_CERT_ADDRESS + RT_WRITE_KEY_OFFSET, sizeof(rt_write_key_info_t), g_write_key_info.write_key);
    if(kStatus_Success != ret_val) return ret_val;
    
    memcpy(write_key, g_write_key_info.write_key, RT_WRITE_KEY_SIZE);
#else
    ret_val = rt_nvm_read(RT_KEY_CERT_ADDRESS + RT_WRITE_KEY_OFFSET, 
                          write_key,
                          RT_WRITE_KEY_SIZE, 
                          NULL);
#endif

    return ret_val;
}


status_t rt_get_latch_key(uint8_t* volatile_key)
{
    status_t ret_val;
#ifdef USE_SEST
    rt_latch_key_info_t g_latch_key_info;

    ret_val = rt_sest_read_data_mac(&ctx, &g_storage, RT_KEY_CERT_ADDRESS + RT_LATCH_KEY_OFFSET, sizeof(rt_latch_key_info_t), g_latch_key_info.latch_key);
    if(kStatus_Success != ret_val) return ret_val;
    
    memcpy(volatile_key, g_latch_key_info.latch_key, RT_LATCH_KEY_SIZE);
#else
    ret_val = rt_nvm_read(RT_KEY_CERT_ADDRESS+RT_LATCH_KEY_OFFSET, 
                          volatile_key,
                          RT_LATCH_KEY_SIZE, 
                          NULL);
#endif
    return ret_val;
}


status_t rt_set_latch(const uint8_t* volatile_key, bool state)
{
    status_t ret_val;
    // set Persistent Latch by Volatile Key
    static uint8_t other_data[] = {0x08, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                   0x00, 0x00, 0x00, 0x00, 0x00}; 
    struct atecc608_checkmac_params checkmac_params;
    ATCA_STATUS status;
    bool latch_state;
    uint8_t scb_num_in[20];
        
    extern status_t rt_rng_vector_generate(uint8_t * const p_target, size_t size);
    ret_val = rt_rng_vector_generate(scb_num_in, 20);
    if(ret_val != kStatus_Success)
    {
        return ret_val;
    }
    //client response from calc
    checkmac_params.slot_number = VOLATILE_KEY;
    checkmac_params.challenge = NULL;
    checkmac_params.response = NULL;
    checkmac_params.other_data = other_data;
    checkmac_params.num_in = scb_num_in;
    checkmac_params.enc_key = volatile_key;
    status = atecc608_mac_check(&checkmac_params);
    memset(&checkmac_params, 0, sizeof(checkmac_params));
    if( ATCA_SUCCESS != status){
        //PRINTF("mac check error");
        return kStatusECCCHKMAC;
    }
  
    status = atecc608_set_latch(state);
    if( ATCA_SUCCESS != status){
        return kStatusECCSETLATCH;
    }  
  
    // check persistlatch
    status = atecc608_get_latch(&latch_state);
    if( ATCA_SUCCESS != status){
        //PRINTF("get latch state error");
        return kStatusECCGETLATCH;
    }
    
    if(state == latch_state)
      return kStatus_Success;
    else
      return kStatusECCCHKLATCH;
}


status_t rt_unlock_keys(void)
{
    status_t ret_val;
    uint8_t volatile_key[32];
    
    ret_val = rt_get_latch_key(volatile_key);
    if(kStatus_Success != ret_val)
    {
      memset(volatile_key, 0, sizeof(volatile_key));  
      return ret_val;
    }

    ret_val = rt_set_latch(volatile_key, LATCH_STATE_SET);
    
    memset(volatile_key, 0, sizeof(volatile_key));  
    return ret_val;
}

status_t rt_block_keys(void)
{
    status_t ret_val;
    uint8_t volatile_key[32];
    
    ret_val = rt_get_latch_key(volatile_key);
    if(kStatus_Success != ret_val)
    {
      memset(volatile_key, 0, sizeof(volatile_key));  
      return ret_val;
    }

    ret_val = rt_set_latch(volatile_key, LATCH_STATE_CLR);

    memset(volatile_key, 0, sizeof(volatile_key));
    return ret_val;
}


status_t rt_atecc_encrypt_read(uint8_t* data_buff, 
                               uint16_t slot, 
                               size_t offset, 
                               size_t length)
{
    ATCA_STATUS ret_code;
    atecc608_rw_enc_params_t rw_params;
    uint8_t enc_read_key[32];
    
    if(kStatus_Success != rt_get_read_key(enc_read_key))
    {
        return kStatusECCKEYError;
    }
    
    rw_params.slot_number = slot;
    rw_params.offset = offset;
    rw_params.buffer = data_buff;
    rw_params.length = length;
    rw_params.enckey = enc_read_key;
    rw_params.enc_slot_id = ENCRYPT_READ_KEY;         //read key slot    
    ret_code = atecc608_read_data_enc(&rw_params);
    if( ATCA_SUCCESS != ret_code )
    {
        memset(enc_read_key, 0, sizeof(enc_read_key));
        return kStatusECCCMDError;
    }

    memset(enc_read_key, 0, sizeof(enc_read_key));
    return kStatus_Success;
}


status_t rt_atecc_encrypt_write(uint8_t* data_buff, 
                               uint16_t slot, 
                               size_t offset, 
                               size_t length)
{
    ATCA_STATUS ret_code;
    uint8_t enc_write_key[32];
    atecc608_rw_enc_params_t rw_params;
    
    if(kStatus_Success != rt_get_write_key(enc_write_key))
    {
        return kStatusECCKEYError;
    }
    
    rw_params.slot_number = slot;
    rw_params.offset = offset;
    rw_params.buffer = data_buff;
    rw_params.length = length; 
    rw_params.enckey = enc_write_key;
    rw_params.enc_slot_id = ENCRYPT_WRITE_KEY;          //write key slot   
    
    ret_code = atecc608_write_data_enc(&rw_params);
    if( ATCA_SUCCESS != ret_code )
    {
        memset(enc_write_key, 0, sizeof(enc_write_key));
        return kStatusECCCMDError;
    }

    memset(enc_write_key, 0, sizeof(enc_write_key));
    return kStatus_Success;
}


status_t _get_version_arg(rt_ver_type type, uint8_t* offset, uint8_t* length)
{
    if((offset == NULL) || (length == NULL))
    {
      return kStatus_InvalidArgument;
    }
  
    switch(type)
    {
    case verKicker:
      *offset = RT_CONF_KICKER_OFFSET;
      *length = 4;
      break;
    
    case verApp:
      *offset = RT_CONF_APP_OFFSET;
      *length = 4;
      break;
      
    case verOTA:
      *offset = RT_CONF_OTA_OFFSET;
      *length = 4;
      break;
      
    case verAll:
      *offset = 0;
      *length = RT_ATECC_VER_SIZE;
      break;

    default:
      return kStatus_InvalidArgument;
      break;
    }
  
    return kStatus_Success;
}


status_t rt_atecc_init_version(void)
{
    status_t ret_val;
    uint8_t ver_buff[RT_ATECC_VER_SIZE+RT_ATECC_CRC_SIZE];
    uint32_t crc;

    memset(ver_buff, 0, RT_ATECC_VER_SIZE);
    
    // Calc CRC
    crc = rt_checksum_crc(ver_buff, RT_ATECC_VER_SIZE);
    memcpy(&ver_buff[RT_ATECC_VER_SIZE], &crc, RT_ATECC_CRC_SIZE);
    
    ret_val = rt_atecc_encrypt_write(ver_buff,
                                     CONF_SLOT,
                                     RT_ATECC_VER_OFFSET, 
                                     sizeof(ver_buff));
    return ret_val;
}


status_t rt_atecc_get_version(uint8_t * ver_data, rt_ver_type type)
{
    status_t ret_val;
    uint8_t ver_buff[RT_ATECC_VER_SIZE+RT_ATECC_CRC_SIZE];
    uint32_t crc_chk;
    uint8_t offset, length;
    
    ret_val = rt_atecc_encrypt_read(ver_buff,
                                    CONF_SLOT,
                                    RT_ATECC_VER_OFFSET, 
                                    sizeof(ver_buff));
    if(kStatus_Success != ret_val) return ret_val;
    
    
    //ECC_DEBUG_HEXDUMP(ver_buff, 32);
   
    // Check CRC
    memcpy(&crc_chk, &ver_buff[RT_ATECC_VER_SIZE], RT_ATECC_CRC_SIZE);
    if(!rt_checksum_crc_ok(ver_buff, RT_ATECC_VER_SIZE, crc_chk))
    {
        ret_val = kStatusECCCHKMAC;
        goto _EXIT;
    }
    
    ret_val = _get_version_arg(type, &offset, &length);
    if(kStatus_Success != ret_val) goto _EXIT;
    
    memcpy(ver_data, &ver_buff[offset], length);

_EXIT:
    memset(ver_buff, 0, sizeof(ver_buff));
    return ret_val;
}


bool rt_atecc_check_version(uint8_t * ver_data, rt_ver_type type)
{
    status_t ret_val;
    uint32_t version;
    uint32_t ver_cur;

    ret_val = rt_atecc_get_version((uint8_t *)&ver_cur, type);
    if(kStatus_Success != ret_val)
    {
        PRINTF("ATECC read version fail!%d\r\n", ret_val);
        return false;
    } 

    memcpy(&version, ver_data, sizeof(version));
    
    if(version >= ver_cur){
      return true;
    }else{
      return false;
    }
}


status_t rt_atecc_set_version(uint8_t * ver_data, rt_ver_type type)
{
    status_t ret_val;
    uint8_t ver_buff[RT_ATECC_VER_SIZE+RT_ATECC_CRC_SIZE];
    uint8_t offset, length;
    uint32_t crc;
  
    ret_val = rt_atecc_get_version(ver_buff, verAll);
    if(kStatus_Success != ret_val) return ret_val;
  
    ret_val = _get_version_arg(type, &offset, &length);
    if(kStatus_Success != ret_val) goto _EXIT;
    
    // Check current version, No need to write if the same
    if(memcmp(&ver_buff[offset], ver_data, length) == 0)
    {
        goto _EXIT;
    }

    memcpy(&ver_buff[offset], ver_data, length);
    
    // Calc CRC
    crc = rt_checksum_crc(ver_buff, RT_ATECC_VER_SIZE);
    memcpy(&ver_buff[RT_ATECC_VER_SIZE], &crc, RT_ATECC_CRC_SIZE);
    
    ret_val = rt_atecc_encrypt_write(ver_buff,
                                     CONF_SLOT,
                                     RT_ATECC_VER_OFFSET, 
                                     sizeof(ver_buff));

_EXIT:
    memset(ver_buff, 0, sizeof(ver_buff));
    return ret_val;
}



#endif
