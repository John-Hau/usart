/** \file
 *  \brief  AES host interface for VSOM Platform
 *
 *  \date   November 2019
 *
 *  \copyright Honeywell
 *  ALL RIGHTS RESERVED, Honeywell Confidential and Proprietary.
 */

#include "aes_host.h"

#include "fsl_bee.h"
#include "fsl_cache.h"

#ifdef FREERTOS
#include "rt_mutex_hal.h"

extern rt_lock_t dcp_lock;
#endif
#include "rng_host.h"
#include "Sha256_host.h"

void reverse_memcpy(const uint8_t* src, uint8_t* dst, uint32_t size)
{
    for(uint32_t i=0;i<size;i++)
    {
        dst[size-1-i] = src[i];
    }
}


static status_t result_get(int error)
{
    status_t ret_val;
    switch (error)
    {
        case 0:
            ret_val = kStatus_Success;
            break;

        case MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH:
            ret_val = kStatusInvalidLENGTH;
            break;

        case MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA:
            ret_val = kStatusInvalidLENGTH;
            break;

        case MBEDTLS_ERR_CIPHER_ALLOC_FAILED:
            ret_val = kStatusInvalidALLOC;
            break;

        case MBEDTLS_ERR_CIPHER_FEATURE_UNAVAILABLE:
            ret_val = kStatusInvalidUNAVAILABLE;
            break;

        case MBEDTLS_ERR_MD_BAD_INPUT_DATA:
            ret_val = kStatus_InvalidArgument;
            break;

        default:
            ret_val = kStatusInvalidINTERNAL;
            break;
    }

    return ret_val;
}


// Helper function to construct iv for aes ctr128
status_t rt_aes_ctr_construct_iv(uint8_t * iv, size_t counter)
{
     uint8_t i;
     uint8_t c=0;
     uint8_t counter_bytes[4];
     uint64_t temp;
    
     memcpy(counter_bytes, &counter, 4);
     for(i=0;i<4;i++)
     {
         temp = iv[15-i] + counter_bytes[i] + c;
         
         if(temp > 0xFF)
         {
             c = 1;
         }
         else
         {
             c = 0;
         }

         iv[15-i] = temp & 0xFF;
     }
  
     if(c)
     {
         for( i = 12; i > 0; i-- )
           if( ++iv[i - 1] != 0 )
             break;
     }
 
    return kStatus_Success;
}


// Currently default use keyslot0 & channel0 

status_t rt_aes_init(void * const    p_context,
                     rt_aes_mode_t   mode,
                     rt_operation_t  operation)
{
    if(p_context == NULL)
    {
        return kStatus_InvalidArgument;
    }
    
    rt_aes_context_t *p_ctx = (rt_aes_context_t *)p_context;
  
    mbedtls_aes_init( &p_ctx->ctx );
    p_ctx->mode = mode;
    p_ctx->operation = operation;
    p_ctx->key_size = RT_BITS_128;
    
    return kStatus_Success;
}


status_t rt_aes_key_set(void * const p_context, uint8_t * p_key)
{
    int mtls_ret;
    status_t ret_val;
    
    if(p_context == NULL)
    {
        return kStatus_InvalidArgument;
    }
    
    rt_aes_context_t *p_ctx = (rt_aes_context_t *)p_context;

    switch(p_ctx->mode)
    {
    case RT_AES_MODE_CBC:
    case RT_AES_MODE_CBC_PAD_PCKS7:
      if (p_ctx->operation == RT_ENCRYPT)
      {
          mtls_ret = mbedtls_aes_setkey_enc(&p_ctx->ctx,
                                            (uint8_t const *)p_key,
                                            p_ctx->key_size);
          
          
          
      }else{
          mtls_ret = mbedtls_aes_setkey_dec(&p_ctx->ctx,
                                            (uint8_t const *)p_key,
                                            p_ctx->key_size);
      }
      break;
      
    case RT_AES_MODE_CTR:
       /* the same key*/
      mtls_ret = mbedtls_aes_setkey_enc(&p_ctx->ctx,
                                         (uint8_t const *)p_key,
                                         p_ctx->key_size);
      break;
    
    case RT_AES_MODE_ECB:
    case RT_AES_MODE_ECB_PAD_PCKS7:
      if (p_ctx->operation == RT_ENCRYPT)
      {
          mtls_ret = mbedtls_aes_setkey_enc(&p_ctx->ctx,
                                            (uint8_t const *)p_key,
                                            p_ctx->key_size);
      }else{
          mtls_ret = mbedtls_aes_setkey_dec(&p_ctx->ctx,
                                            (uint8_t const *)p_key,
                                            p_ctx->key_size);
      }
      break;
      
     default:
      return kStatusInvalidUNAVAILABLE; 
    }

    ret_val = result_get(mtls_ret);
    return ret_val;
}


status_t rt_aes_iv_set(void * const p_context, uint8_t * p_iv)
{
    if(p_context == NULL)
    {
        return kStatus_InvalidArgument;
    }
    rt_aes_context_t *p_ctx = (rt_aes_context_t *)p_context;

    memcpy(&p_ctx->iv[0], p_iv, RT_AES_IV_SIZE);

    return kStatus_Success;
}


static int bigdata_mbedtls_ecb_crypt(rt_aes_context_t * const p_ctx,
                                     uint8_t *          p_text_in,
                                     uint8_t *          p_text_out,
                                     size_t             text_size)
{
    int    error        = 0;
    size_t crypted_text = 0;

    if ((text_size & 0x0F) != 0)
    {
        return MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH;
    }

    while (crypted_text < text_size)
    {
        error = mbedtls_aes_crypt_ecb(&p_ctx->ctx,
                                      (int)p_ctx->operation,
                                      p_text_in  + crypted_text,
                                      p_text_out + crypted_text);
        if (error != 0)
        {
            break;
        }
        crypted_text += RT_AES_BLOCK_SIZE;
    }

    return error;
}


status_t rt_aes_update(void * const p_context,
                       uint8_t * p_data_in,
                       size_t    data_size,
                       uint8_t * p_data_out)
{
    int mtls_ret;
    status_t ret_val;
    
    if(p_context == NULL)
    {
        return kStatus_InvalidArgument;
    }
    rt_aes_context_t *p_ctx = (rt_aes_context_t *)p_context;
    
    if (((data_size & 0xF) != 0) && (p_ctx->mode != RT_AES_MODE_CTR))
    {
        return kStatusInvalidLENGTH;
    }

    switch(p_ctx->mode)
    {
    case RT_AES_MODE_CBC:
    case RT_AES_MODE_CBC_PAD_PCKS7:
      mtls_ret = mbedtls_aes_crypt_cbc( &p_ctx->ctx, 
                                         p_ctx->mode , 
                                         data_size, 
                                         p_ctx->iv, 
                                         (uint8_t const *)p_data_in, 
                                         p_data_out);
      break;
      
    case RT_AES_MODE_CTR:
      {
      size_t  nc_off = 0;
      uint8_t stream_block[RT_AES_BLOCK_SIZE];
      
      mtls_ret = mbedtls_aes_crypt_ctr( &p_ctx->ctx, 
                                         data_size, 
                                         &nc_off, 
                                         p_ctx->iv,
                                         stream_block,
                                         (uint8_t const *)p_data_in, 
                                         p_data_out );
      }
      break;
    
    case RT_AES_MODE_ECB:
    case RT_AES_MODE_ECB_PAD_PCKS7:
      mtls_ret = bigdata_mbedtls_ecb_crypt( p_ctx, 
                                            p_data_in, 
                                            p_data_out, 
                                            data_size);
      break;
      
     default:
      return kStatusInvalidUNAVAILABLE; 
    }

    ret_val = result_get(mtls_ret);
    return ret_val;
}


static status_t rt_aes_uninit(rt_aes_context_t *p_context)
{
    if(p_context == NULL)
    {
        return kStatus_InvalidArgument;
    }
  
    mbedtls_aes_free(&p_context->ctx);
    memset(p_context, 0, sizeof(rt_aes_context_t));

    return kStatus_Success;
}



status_t rt_aes_finalize(void *         const p_context,
                         uint8_t *      p_data_in,
                         size_t         data_size,
                         uint8_t *      p_data_out,
                         size_t *       p_data_out_size)
{
    status_t ret_val;
    
    if(p_context == NULL)
    {
        return kStatus_InvalidArgument;
    }

    rt_aes_context_t *p_ctx = (rt_aes_context_t *)p_context;
    if(data_size > 0)
    {
        ret_val = rt_aes_update(p_ctx, p_data_in, data_size, p_data_out);
        if(kStatus_Success != ret_val) return ret_val;
    
        *p_data_out_size = data_size;
    }
    
    ret_val = rt_aes_uninit(p_ctx);
    
    return ret_val;
}


status_t rt_aes_crypt(rt_aes_mode_t   mode,
                      rt_operation_t  operation,
                      uint8_t *       p_key,
                      uint8_t *       p_iv,
                      uint8_t *       p_data_in,
                      size_t          data_size,
                      uint8_t *       p_data_out,
                      size_t *        p_data_out_size)
{
    status_t ret_val;
    rt_aes_context_t aes_ctx;
  
    if((p_key == NULL) || (p_data_in == NULL) || (p_data_out == NULL) ||
                          (data_size == 0))
    {
        return kStatus_InvalidArgument;
    }
    
    ret_val = rt_aes_init(&aes_ctx, mode, operation);
    if(kStatus_Success != ret_val) return ret_val;
    
    ret_val = rt_aes_key_set(&aes_ctx, p_key);
    if(kStatus_Success != ret_val) return ret_val;

    if(p_iv != NULL){
        ret_val = rt_aes_iv_set(&aes_ctx, p_iv);
        if(kStatus_Success != ret_val) return ret_val;
    }
    
    ret_val = rt_aes_finalize(&aes_ctx, 
                              p_data_in, 
                              data_size, 
                              p_data_out, 
                              p_data_out_size);
    return ret_val;
}


//////////////////////////////For RT///////////////////////////////////////////
// DCP
#if RT_DCP_USE_OTP_KEY

status_t DCP_OTPKeySelect(dcp_otp_key_select keySelect)
{
    if (keySelect == kDCP_OTPMKKeyLow)
    {
        IOMUXC_GPR->GPR3 &= ~(1 << IOMUXC_GPR_GPR3_DCP_KEY_SEL_SHIFT);
        IOMUXC_GPR->GPR10 &= ~(1 << IOMUXC_GPR_GPR10_DCPKEY_OCOTP_OR_KEYMUX_SHIFT);
    }

    else if (keySelect == kDCP_OTPMKKeyHigh)
    {
        IOMUXC_GPR->GPR3 |= (1 << IOMUXC_GPR_GPR3_DCP_KEY_SEL_SHIFT);
        IOMUXC_GPR->GPR10 &= ~(1 << IOMUXC_GPR_GPR10_DCPKEY_OCOTP_OR_KEYMUX_SHIFT);
    }

    else if (keySelect == kDCP_OCOTPKeyLow)
    {
        IOMUXC_GPR->GPR3 &= ~(1 << IOMUXC_GPR_GPR3_DCP_KEY_SEL_SHIFT);
        IOMUXC_GPR->GPR10 |= (1 << IOMUXC_GPR_GPR10_DCPKEY_OCOTP_OR_KEYMUX_SHIFT);
    }

    else if (keySelect == kDCP_OCOTPKeyHigh)
    {
        IOMUXC_GPR->GPR3 |= (1 << IOMUXC_GPR_GPR3_DCP_KEY_SEL_SHIFT);
        IOMUXC_GPR->GPR10 |= (1 << IOMUXC_GPR_GPR10_DCPKEY_OCOTP_OR_KEYMUX_SHIFT);
    }

    else
    {
        return kStatus_InvalidArgument;
    }

    return kStatus_Success;
}
#endif

status_t rt_aes_ctr(dcp_otp_key_select keySelect,
                        uint8_t *       p_key,
                        uint8_t *       p_iv,
                        uint8_t *       p_data_in,
                        size_t          data_size,
                        uint8_t *       p_data_out)
{
    status_t ret_val;
    int c, i;
    uint8_t stream_block[RT_AES_BLOCK_SIZE];
    size_t n = 0;
    size_t length = data_size;
    uint8_t nonce_counter[RT_AES_IV_SIZE];
    uint8_t *input = p_data_in;
    uint8_t *output = p_data_out;
    dcp_handle_t m_handle;
    dcp_channel_t channel = kDCP_Channel3;
    dcp_key_slot_t slot = kDCP_KeySlot0;
    
    if((p_iv == NULL) || (p_data_in == NULL) || (p_data_out == NULL) ||
                          (data_size == 0))
    {
        return kStatus_InvalidArgument;
    }

    if ( n > 0x0F )
        return kStatus_InvalidArgument;

    if(keySelect != kDCP_None)
    {
        slot = kDCP_OtpKey;
        m_handle.swapConfig = kDCP_KeyByteSwap | kDCP_KeyWordSwap;
    }else{
        m_handle.swapConfig = kDCP_NoSwap;
    }
    
    m_handle.channel    = channel;
    m_handle.keySlot = slot;
    
    #ifdef FREERTOS
    ret_val = rt_lock_mutex(&dcp_lock);
    if(kStatus_Success != ret_val) return ret_val;
    #endif

    ret_val = DCP_AES_SetKey(DCP, &m_handle, p_key, 16);
    if(kStatus_Success != ret_val)
    {
        #ifdef FREERTOS
        rt_unlock_mutex(&dcp_lock);
        #endif
        return ret_val;
    }

    memcpy(nonce_counter, p_iv, RT_AES_IV_SIZE);    
    while( length-- )
    {
        if( n == 0 ) {
            ret_val = DCP_AES_EncryptEcb(DCP, &m_handle, nonce_counter, stream_block, 16);
            if(kStatus_Success != ret_val)
            {
                #ifdef FREERTOS
                rt_unlock_mutex(&dcp_lock);
                #endif
                return ret_val;
            }

            for( i = 16; i > 0; i-- )
                if( ++nonce_counter[i - 1] != 0 )
                    break;
        }
        c = *input++;
        *output++ = (unsigned char)( c ^ stream_block[n] );

        n = ( n + 1 ) & 0x0F;
    }
    
    #ifdef FREERTOS
    rt_unlock_mutex(&dcp_lock);
    #endif

    return kStatus_Success;
}


/*
 * Parse PRDBx and get counter
*/
status_t rt_get_prdb_ctr(uint8_t idx, uint8_t* counter)
{
    status_t ret_val;
    uint32_t ekib;
    uint32_t eprdb;
    uint8_t kib_data[32];
    uint32_t prdb_en_data[20];
    uint32_t i;
    
    dcp_handle_t m_handle;
    
    if(idx)
    {
      ekib = 0x60000800;
      eprdb = 0x60000880;
    }else{
      ekib = 0x60000400;
      eprdb = 0x60000480;
    }
    
    // decode ekib
    memcpy(kib_data, (uint8_t*)ekib, sizeof(kib_data));
    for(i=0;i<sizeof(kib_data);i++)
    {
        if(kib_data[i] != 0xff) break;
    }
    
    if(i == sizeof(kib_data))
    {
        PRINTF("No EKIB\r\n");
        return kStatus_Success;
    }
    
    m_handle.channel    = kDCP_Channel3;
    m_handle.swapConfig = kDCP_KeyByteSwap | kDCP_KeyWordSwap;
    m_handle.keySlot = kDCP_OtpKey;
    
    ret_val = DCP_AES_SetKey(DCP, &m_handle, NULL, 0);
    if(kStatus_Success != ret_val) return ret_val;
    
    ret_val = DCP_AES_DecryptEcb(DCP, &m_handle, kib_data, kib_data, sizeof(kib_data));
    if(kStatus_Success != ret_val) return ret_val;
    
    // decode eprdb
    memcpy(prdb_en_data, (uint8_t*)eprdb, sizeof(prdb_en_data));
    
    m_handle.channel    = kDCP_Channel0;
    m_handle.swapConfig = kDCP_NoSwap;
    m_handle.keySlot = kDCP_KeySlot0;
    ret_val = DCP_AES_SetKey(DCP, &m_handle, (const uint8_t*)kib_data, 16);
    if(kStatus_Success != ret_val) return ret_val;
    
    ret_val = DCP_AES_DecryptCbc(DCP, 
                                 &m_handle, 
                                 (uint8_t*)prdb_en_data, 
                                 (uint8_t*)prdb_en_data, 
                                 sizeof(prdb_en_data), 
                                 &kib_data[16]);
    if(kStatus_Success != ret_val) return ret_val;
    
    if((prdb_en_data[0] != 0x5F474154U) || 
        (prdb_en_data[1] != 0x52444845U) || 
          (prdb_en_data[2] != 0x56010000U))
    {
        return kStatus_InvalidArgument;
    }

    // Copy counter    
    reverse_memcpy((uint8_t*)&prdb_en_data[8], (uint8_t *)counter, 16);
    memset(counter+12, 0, 4);
    
    return kStatus_Success;
}


// BEE
#if 1
// TODO: Just for test
extern uint8_t app_key_test_bee[16];
#endif

void bee_setkey_valid(BEE_Type *base, bee_region_t region)
{
    /* Wait until BEE is in idle state */
    while (0U == (BEE_GetStatusFlags(base) & (uint32_t)kBEE_IdleFlag)) {}

    /* Clear KEY_VALID bit before new key is loaded */
    base->CTRL &= ~BEE_CTRL_KEY_VALID_MASK;

    if (region == kBEE_Region0)
    {
        base->CTRL &= ~BEE_CTRL_KEY_REGION_SEL_MASK;
    }else if (region == kBEE_Region1){
        base->CTRL |= BEE_CTRL_KEY_REGION_SEL_MASK;
    }

    /* Set KEY_VALID bit to trigger key loading */
    base->CTRL |= BEE_CTRL_KEY_VALID_MASK;

    /* Wait until key is ready */
    while (0U == (base->CTRL & BEE_CTRL_KEY_VALID_MASK)) {}
}

#if RT_NONCE_DYNAMIC

status_t bee_set_facx(uint32_t start, uint32_t end, uint8_t idx)
{
    switch(idx)
    {
    case 0:
      IOMUXC_GPR->GPR18 = start;
      IOMUXC_GPR->GPR19 = end;
      IOMUXC_GPR->GPR11 |= IOMUXC_GPR_GPR11_BEE_DE_RX_EN(1);
      break;
    case 1:
      IOMUXC_GPR->GPR20 = start;
      IOMUXC_GPR->GPR21 = end;
      IOMUXC_GPR->GPR11 |= IOMUXC_GPR_GPR11_BEE_DE_RX_EN(2);
      break;
    case 2:
      IOMUXC_GPR->GPR22 = start;
      IOMUXC_GPR->GPR23 = end;
      IOMUXC_GPR->GPR11 |= IOMUXC_GPR_GPR11_BEE_DE_RX_EN(4);
      break;
    case 3:
      IOMUXC_GPR->GPR24 = start;
      IOMUXC_GPR->GPR25 = end;
      IOMUXC_GPR->GPR11 |= IOMUXC_GPR_GPR11_BEE_DE_RX_EN(8);
      break;

    default: return kStatus_InvalidArgument;
    }

    return kStatus_Success;
}


status_t bee_set_fac(uint32_t start, uint32_t end)
{
    if(start >= (IOMUXC_GPR->GPR18) && start < (IOMUXC_GPR->GPR19))
    {
        bee_set_facx(start, end, 0);
        return kStatus_Success;
    }

    if(start >= (IOMUXC_GPR->GPR20) && start < (IOMUXC_GPR->GPR21))
    {
        bee_set_facx(start, end, 1);
        return kStatus_Success;
    }

    if(start >= (IOMUXC_GPR->GPR22) && start < (IOMUXC_GPR->GPR23))
    {
        bee_set_facx(start, end, 2);
        return kStatus_Success;
    }

    if(start >= (IOMUXC_GPR->GPR24) && start < (IOMUXC_GPR->GPR25))
    {
        bee_set_facx(start, end, 3);
        return kStatus_Success;
    }

    if(0 == (IOMUXC_GPR->GPR24) && 0 == (IOMUXC_GPR->GPR25))
    {
        bee_set_facx(start, end, 3);
        return kStatus_Success;
    }

    if(0 == (IOMUXC_GPR->GPR22) && 0 == (IOMUXC_GPR->GPR23))
    {
        bee_set_facx(start, end, 2);
        return kStatus_Success;
    }

    if(0 == (IOMUXC_GPR->GPR20) && 0 == (IOMUXC_GPR->GPR21))
    {
        bee_set_facx(start, end, 1);
        return kStatus_Success;
    }

    if(0 == (IOMUXC_GPR->GPR18) && 0 == (IOMUXC_GPR->GPR19))
    {
        bee_set_facx(start, end, 0);
        return kStatus_Success;
    }

    return kStatus_Success;
}


void BEE_config(uint32_t start, uint32_t end, uint32_t* counter)
{
    PRINTF("Config BEE\r\n");

    uint8_t disabled;
    uint32_t bee_ctr[4];
    uint32_t bee_key[4];

     /* Read the BEE_KEY1_SEL value */
    uint32_t bee_key_sel1 = OCOTP->CFG5 & 0x0000C000;

    if( bee_key_sel1 == 0x00000000)
      PRINTF("BEE_KEY1 from register\r\n");
    else if( bee_key_sel1 == 0x00008000)
      PRINTF("BEE_KEY1 from OTPMK[256:128]\r\n");
    else if( bee_key_sel1 == 0x00004000)
      PRINTF("BEE_KEY1 from OTPMK[127:0]\r\n");
    else if ( bee_key_sel1 == 0x0000C000)
      PRINTF("BEE_KEY1 from SW-GP2\r\n");

    if(BEE->CTRL & BEE_CTRL_BEE_ENABLE_MASK)
    {
        disabled = 0;
    }else{
        disabled = 1;
    }

    bee_region_config_t beeConfig;
    BEE_GetDefaultConfig(&beeConfig);

    beeConfig.region1Mode = kBEE_AesCtrMode;
    beeConfig.region1Bot = start;
    beeConfig.region1Top = end;

    /* Configure Start address and end address of access protected region */
    /*
    PRINTF("Before GPR18 GPR19 GPR20 GPR21 GPR22 GPR23 GPR24 GPR25:%x %x %x %x %x %x %x %x\r\n",
              IOMUXC_GPR->GPR18, IOMUXC_GPR->GPR19,
              IOMUXC_GPR->GPR20, IOMUXC_GPR->GPR21,
              IOMUXC_GPR->GPR22, IOMUXC_GPR->GPR23,
              IOMUXC_GPR->GPR24, IOMUXC_GPR->GPR25);
    */
    if(kStatus_Success != bee_set_fac(start, end))
    {
        PRINTF("Config BEE fail!\r\n");
        return;
    }
    /*
    PRINTF("After GPR18 GPR19 GPR20 GPR21 GPR22 GPR23 GPR24 GPR25:%x %x %x %x %x %x %x %x\r\n",
              IOMUXC_GPR->GPR18, IOMUXC_GPR->GPR19,
              IOMUXC_GPR->GPR20, IOMUXC_GPR->GPR21,
              IOMUXC_GPR->GPR22, IOMUXC_GPR->GPR23,
              IOMUXC_GPR->GPR24, IOMUXC_GPR->GPR25);
    */
    if(disabled)
    {
        /* Init BEE driver and apply the configuration */
        BEE_Init(BEE);
    }else{
        uint32_t ctr0 = BEE->CTRL;

        beeConfig.region0Mode       = (bee_aes_mode_t)((ctr0 & BEE_CTRL_CTRL_AES_MODE_R0_MASK)>>
                                                       BEE_CTRL_CTRL_AES_MODE_R0_SHIFT);
        beeConfig.region0AddrOffset = BEE->ADDR_OFFSET0;
        beeConfig.region1AddrOffset = BEE->ADDR_OFFSET1;
        beeConfig.region0SecLevel   = (bee_security_level)((ctr0 & BEE_CTRL_SECURITY_LEVEL_R0_MASK)>>
                                                      BEE_CTRL_SECURITY_LEVEL_R0_SHIFT);
        beeConfig.region1SecLevel   = (bee_security_level)((ctr0 & BEE_CTRL_SECURITY_LEVEL_R1_MASK)>>
                                                      BEE_CTRL_SECURITY_LEVEL_R1_SHIFT);
        beeConfig.accessPermission  = (bee_ac_prot_enable)((ctr0 & BEE_CTRL_AC_PROT_EN_MASK)>>
                                                      BEE_CTRL_AC_PROT_EN_SHIFT);
        beeConfig.endianSwapEn      = (bee_endian_swap_enable)((ctr0 & BEE_CTRL_LITTLE_ENDIAN_MASK)>>
                                                      BEE_CTRL_LITTLE_ENDIAN_SHIFT);
    }

    BEE_SetConfig(BEE, &beeConfig);
    DCACHE_InvalidateByRange(start, end + 1 - start);
    ICACHE_InvalidateByRange(start, end + 1 - start);

    if( bee_key_sel1 == 0x00000000)
    {
        reverse_memcpy((uint8_t *)app_key_test_bee, (uint8_t *)bee_key, 16);
        BEE_SetRegionKey(BEE, kBEE_Region1, (uint8_t *)bee_key, 16);
    }else{
        bee_setkey_valid(BEE, kBEE_Region1);
    }

    //Big endian, ctr lower 32bit must be 0
    reverse_memcpy((uint8_t *)counter, (uint8_t *)bee_ctr, 16);
    bee_ctr[0] = 0;

    BEE_SetRegionNonce(BEE, kBEE_Region1, (uint8_t *)bee_ctr, 16);
    BEE_Enable(BEE);

    memset((uint8_t *)bee_key, 0 , 16);
    memset((uint8_t *)bee_ctr, 0 , 16);
}

#else
void BEE_config(uint32_t start, uint32_t end, uint32_t* counter)
{
    PRINTF("Config BEE\r\n");
    
    uint8_t disabled;
    uint32_t bee_ctr[4];
    uint32_t bee_key[4];

     /* Read the BEE_KEY1_SEL value */
    uint32_t bee_key_sel1 = OCOTP->CFG5 & 0x0000C000;

    if( bee_key_sel1 == 0x00000000)
      PRINTF("BEE_KEY1 from register\r\n");
    else if( bee_key_sel1 == 0x00008000)
      PRINTF("BEE_KEY1 from OTPMK[256:128]\r\n");
    else if( bee_key_sel1 == 0x00004000)
      PRINTF("BEE_KEY1 from OTPMK[127:0]\r\n");
    else if ( bee_key_sel1 == 0x0000C000)
      PRINTF("BEE_KEY1 from SW-GP2\r\n");
    
    if(BEE->CTRL & BEE_CTRL_BEE_ENABLE_MASK)
    {
        disabled = 0;
    }else{
        disabled = 1;
    }

    bee_region_config_t beeConfig;
    BEE_GetDefaultConfig(&beeConfig);
  
    beeConfig.region1Mode = kBEE_AesCtrMode;
    beeConfig.region1Bot = start;
    beeConfig.region1Top = end;

    /* Configure Start address and end address of access protected region */
    /*
    PRINTF("GPR18 GPR19 GPR20 GPR21 GPR22 GPR23 GPR24 GPR25:%x %x %x %x %x %x %x %x\r\n", 
              IOMUXC_GPR->GPR18, IOMUXC_GPR->GPR19, 
              IOMUXC_GPR->GPR20, IOMUXC_GPR->GPR21,
              IOMUXC_GPR->GPR22, IOMUXC_GPR->GPR23,
              IOMUXC_GPR->GPR24, IOMUXC_GPR->GPR25);
    */
    //IOMUXC_GPR->GPR20 = start;
    //IOMUXC_GPR->GPR21 = end;
    /* Enable BEE data decryption of memory region-1 */
    //IOMUXC_GPR->GPR11 |= IOMUXC_GPR_GPR11_BEE_DE_RX_EN(2);
    
    IOMUXC_GPR->GPR18 = start;
    IOMUXC_GPR->GPR19 = end;
    IOMUXC_GPR->GPR11 |= IOMUXC_GPR_GPR11_BEE_DE_RX_EN(1);
    
    
    if(disabled)
    {
        /* Init BEE driver and apply the configuration */
        BEE_Init(BEE);
    }else{
        uint32_t ctr0 = BEE->CTRL;
        
        beeConfig.region0Mode       = (bee_aes_mode_t)((ctr0 & BEE_CTRL_CTRL_AES_MODE_R0_MASK)>>
                                                       BEE_CTRL_CTRL_AES_MODE_R0_SHIFT);
        beeConfig.region0AddrOffset = BEE->ADDR_OFFSET0;
        beeConfig.region1AddrOffset = BEE->ADDR_OFFSET1;
        beeConfig.region0SecLevel   = (bee_security_level)((ctr0 & BEE_CTRL_SECURITY_LEVEL_R0_MASK)>>
                                                      BEE_CTRL_SECURITY_LEVEL_R0_SHIFT);
        beeConfig.region1SecLevel   = (bee_security_level)((ctr0 & BEE_CTRL_SECURITY_LEVEL_R1_MASK)>>
                                                      BEE_CTRL_SECURITY_LEVEL_R1_SHIFT);
        beeConfig.accessPermission  = (bee_ac_prot_enable)((ctr0 & BEE_CTRL_AC_PROT_EN_MASK)>>
                                                      BEE_CTRL_AC_PROT_EN_SHIFT);
        beeConfig.endianSwapEn      = (bee_endian_swap_enable)((ctr0 & BEE_CTRL_LITTLE_ENDIAN_MASK)>>
                                                      BEE_CTRL_LITTLE_ENDIAN_SHIFT);
    }
    
    BEE_SetConfig(BEE, &beeConfig);
    DCACHE_InvalidateByRange(start, end + 1 - start);
    ICACHE_InvalidateByRange(start, end + 1 - start);    
  
    if( bee_key_sel1 == 0x00000000)
    {
        reverse_memcpy((uint8_t *)app_key_test_bee, (uint8_t *)bee_key, 16);
        BEE_SetRegionKey(BEE, kBEE_Region1, (uint8_t *)bee_key, 16);
    }else{
        bee_setkey_valid(BEE, kBEE_Region1);
    }
    
    //Big endian, ctr lower 32bit must be 0
    reverse_memcpy((uint8_t *)counter, (uint8_t *)bee_ctr, 16);
    bee_ctr[0] = 0;
    
    BEE_SetRegionNonce(BEE, kBEE_Region1, (uint8_t *)bee_ctr, 16);
    BEE_Enable(BEE);
    
    memset((uint8_t *)bee_key, 0 , 16);
    memset((uint8_t *)bee_ctr, 0 , 16);
}
#endif

//need define dcp_lock (like this : rt_lock_t dcp_lock;) before use RT_Enc_Hash_Data or RT_Dec_Verify_Data  if #define FREERTOS
// AES CRT 128   using OTP key.
// in_plaintext : input data pointer need to encryption
// in_length    : length of input data, in_length%16 need equel 0;
// out_cipher   : three part : ciphertext + in_plaintext's hash value(if en_integrity == true) + iv
// *out_length  : the length of output data :
//                *out_length need >= in_length + 32(length of hash) + 16(IV length) when en_integrity = true;
//                *out_length need >= in_length + 16(IV length) when en_integrity = false;
// en_integrity : whether need enable hash  : true or false
//return kStatus_Success when run success.

status_t RT_Enc_Hash_Data(const uint8_t *in_plaintext, uint32_t in_length, uint8_t *out_cipher, uint32_t *out_length, bool en_integrity)
{
    status_t ret_val  = kStatus_Success;
    uint8_t iv[RT_AES_IV_SIZE];

    //check input parameter
    if((in_plaintext == NULL) || (in_length == 0) || (in_length%16 != 0) || (out_length == NULL) || (out_cipher == NULL))
    {
        return kStatus_InvalidArgument;
    }

    //check input parameter  *out_length:
    //*out_length need >= in_length + 32(length of hash) + 16(IV length) when en_integrity = true;
    //*out_length need >= in_length + 16(IV length) when en_integrity = false;
    if(((true == en_integrity) && (*out_length < in_length + 48)) || ((false == en_integrity) && (*out_length < in_length + 16)))
    {
        return kStatus_InvalidArgument;
    }

#ifdef FREERTOS
    //init dcp_lock if don't init.
    if((NULL == &dcp_lock) || (NULL == (&dcp_lock)->lock_mtx))
    {
        ret_val = rt_init_mutex(&dcp_lock);
        if(kStatus_Success != ret_val) return ret_val;
    }
#endif

    //generate iv
    ret_val = rt_rng_vector_generate(iv, sizeof(iv));
    if(kStatus_Success != ret_val) return ret_val;

    //AES128 CRT encryption,
    if(DCP_KEY == kDCP_None)   //use OTP key
        return kStatus_InvalidArgument;
    ret_val = rt_aes_ctr(DCP_KEY, NULL, iv, (uint8_t *)in_plaintext, in_length, out_cipher);
    if(kStatus_Success != ret_val) return ret_val;

    *out_length = in_length;

    //add hash: sha256
    if(true == en_integrity)
    {
        size_t digest_len = 32;
        ret_val = rt_hash256_calculate(in_plaintext, in_length, &out_cipher[*out_length], &digest_len);
        if(kStatus_Success != ret_val) return ret_val;

        *out_length += digest_len;
    }

    //copy nonce
    memcpy(&out_cipher[*out_length], iv, RT_AES_IV_SIZE);
    *out_length += RT_AES_IV_SIZE;

    return ret_val;
}

//need define dcp_lock (like this : rt_lock_t dcp_lock;) before use RT_Enc_Hash_Data or RT_Dec_Verify_Data if #define FREERTOS
// AES CRT 128   using OTP key
// in_cipher    : three part : ciphertext + plaintext's hash value(if en_integrity == true) + iv
// in_length    : length of in_cipher, in_length%16 need equel 0;
// out_plaintext   : plaintext
// *out_length  : the length of out_plaintext
//                *out_length need >= in_length - 32(length of hash) - 16(IV length) when en_integrity = true;
//                *out_length need >= in_length - 16(IV length) when en_integrity = false;
// en_integrity : whether need enable hash.
// return kStatus_Success when run success and hash is ok,
// return kStatus_InvalidHash when hash is error.

status_t RT_Dec_Verify_Data(const uint8_t *in_cipher, uint32_t in_length, uint8_t  *out_plaintext, uint32_t *out_length, bool en_integrity)
{
    status_t ret_val  = kStatus_Success;
    uint8_t iv[RT_AES_IV_SIZE];

    //check input parameter
    if((in_cipher == NULL) || (in_length == 0) || (in_length%16 != 0) || (out_length == NULL) || (out_plaintext == NULL))
    {
        return kStatus_InvalidArgument;
    }

    //check in_length and *out_length
    //*out_length need >= in_length - 32(length of hash) - 16(IV length) when en_integrity = true;
    //in_length need > 32(length of hash) + 16(IV length)when en_integrity = true;
    //*out_length need >= in_length - 16(IV length) when en_integrity = false;
    //in_length need > 16(IV length) when en_integrity = false;
    //*out_length need >= 16.
    if(((true == en_integrity) && ((in_length <= 48) || (*out_length < in_length - 48))) ||
		((false == en_integrity) && ((in_length <= 16) || (*out_length < in_length - 16))) ||
		(*out_length < 16))
    {
        return kStatus_InvalidArgument;
    }

#ifdef FREERTOS
    //init dcp_lock.
    if((NULL == &dcp_lock) || (NULL == (&dcp_lock)->lock_mtx))
    {
        ret_val = rt_init_mutex(&dcp_lock);
        if(kStatus_Success != ret_val) return ret_val;
    }
#endif

    //get iv
    memcpy(iv, &in_cipher[in_length - RT_AES_IV_SIZE], RT_AES_IV_SIZE);
    *out_length = in_length - RT_AES_IV_SIZE;

    if(true == en_integrity)
    {
        //32 = the length of hash
        *out_length -= 32;
    }

    //AES128 CRT decode
    if(DCP_KEY == kDCP_None)  //use OTP key
        return kStatus_InvalidArgument;
    ret_val = rt_aes_ctr(DCP_KEY, NULL, iv, (uint8_t *)in_cipher, *out_length, out_plaintext);
    if(kStatus_Success != ret_val) return ret_val;

    //calculate and verify hash
    if(true == en_integrity)
    {
        //32 = the length of hash
        uint8_t digest[32];
        size_t digest_len = 32;

        ret_val = rt_hash256_calculate(out_plaintext, in_length - RT_AES_IV_SIZE - 32, digest, &digest_len);
        if(kStatus_Success != ret_val) return ret_val;

        if(memcmp(digest, &in_cipher[in_length - RT_AES_IV_SIZE - 32], 32) != 0)
        {
            PRINTF("Verify Hash fail!\r\n");
            return kStatus_Fail;
        }
    }

    return ret_val;
}

///////////////////////////////////////////////////////////////////////////////

status_t rt_aes_test(void)
{
    unsigned char aes_test_ctr_key[16] = { 0x76, 0x91, 0xBE, 0x03, 0x5E, 0x50, 0x20, 0xA8,
                                           0xAC, 0x6E, 0x61, 0x85, 0x29, 0xF9, 0xA0, 0xDC };
    unsigned char aes_test_ctr_pt[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                                        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                                        0x20, 0x21, 0x22, 0x23 };
    unsigned char aes_test_ctr_ct[] = { 0xC1, 0xCF, 0x48, 0xA8, 0x9F, 0x2F, 0xFD, 0xD9,
                                        0xCF, 0x46, 0x52, 0xE9, 0xEF, 0xDB, 0x72, 0xD7,
                                        0x45, 0x40, 0xA4, 0x2B, 0xDE, 0x6D, 0x78, 0x36,
                                        0xD5, 0x9A, 0x5C, 0xEA, 0xAE, 0xF3, 0x10, 0x53,
                                        0x25, 0xB2, 0x07, 0x2F };
    unsigned char aes_test_ctr_nonce_counter[16] = { 0x00, 0xE0, 0x01, 0x7B, 0x27, 0x77, 0x7F, 0x3F,
                                                     0x4A, 0x17, 0x86, 0xF0, 0x00, 0x00, 0x00, 0x01 };
    unsigned char buf[64] = {0};
    status_t ret_val;
    size_t out_size = sizeof(aes_test_ctr_ct);
    
    ret_val = rt_aes_crypt(RT_AES_MODE_CTR,
                           RT_ENCRYPT,
                           aes_test_ctr_key,
                           aes_test_ctr_nonce_counter,
                           aes_test_ctr_pt,
                           sizeof(aes_test_ctr_pt),
                           buf,
                           &out_size);
    if(kStatus_Success != ret_val) return ret_val;

    if(memcmp(aes_test_ctr_ct, buf, out_size) != 0)
    {
        PRINTF("AES CTR encrypt fail!\r\n");
    }else{
        PRINTF("AES CTR encrypt pass!\r\n");
    }
    
    ret_val = rt_aes_crypt(RT_AES_MODE_CTR,
                           RT_DECRYPT,
                           aes_test_ctr_key,
                           aes_test_ctr_nonce_counter,
                           aes_test_ctr_ct,
                           sizeof(aes_test_ctr_ct),
                           buf,
                           &out_size);
    if(kStatus_Success != ret_val) return ret_val;    
    
    if(memcmp(aes_test_ctr_pt, buf, out_size) != 0)
    {
        PRINTF("AES CTR decrypt fail!\r\n");
    }else{
        PRINTF("AES CTR decrypt pass!\r\n");
    }  
    
    return kStatus_Success;
}


status_t RT_Enc_Dec_Verify_Data_Test(void)
{
    status_t ret_val = kStatus_Success;
    static const uint8_t plainAes128[] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                                          0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
                                          0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                                          0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};

    uint8_t cipherAes128[sizeof(plainAes128) + 32 + 16];  //32 = length of hash ; 16 = length of nonce;
    uint32_t len_cipherAes128;

    uint8_t output[sizeof(plainAes128)];
    uint32_t len_output;

//  1. input Parameters error
    PRINTF("\r\n1. Input Parameters error test :\r\n");

    PRINTF("\r\n    RT_Enc_Hash_Data(const uint8_t *in_plaintext, uint32_t in_length, uint8_t *out_cipher, uint32_t *out_length, bool en_integrity) test :\r\n");

    //(1). in_plaintext Err (RT_Enc_Hash_Data) en_integrity = true
    PRINTF("\r\n    (1). in_plaintext Err test :    (en_integrity = true) \r\n");
    len_output = 32;
    len_cipherAes128 = sizeof(plainAes128) + 32 + 16;
    ret_val = RT_Enc_Hash_Data(NULL, sizeof(plainAes128), cipherAes128, &len_cipherAes128, true);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(2). in_plaintext Err (RT_Enc_Hash_Data) en_integrity = false
    PRINTF("\r\n    (2). in_plaintext Err test :    (en_integrity = false) \r\n");
    len_output = 32;
    len_cipherAes128 = sizeof(plainAes128) + 32 + 16;
    ret_val = RT_Enc_Hash_Data(NULL, sizeof(plainAes128), cipherAes128, &len_cipherAes128, false);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }
	
    //(3). in_length Err (RT_Enc_Hash_Data) en_integrity = true
    PRINTF("\r\n    (3). in_length Err test :    (en_integrity = true) \r\n");
    len_cipherAes128 = sizeof(plainAes128) + 32 + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128) - 1, cipherAes128, &len_cipherAes128, true);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(4). in_length Err (RT_Enc_Hash_Data) en_integrity = false
    PRINTF("\r\n    (4). in_length Err test :    (en_integrity = false) \r\n");
    len_cipherAes128 = sizeof(plainAes128) + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128) - 1, cipherAes128, &len_cipherAes128, false);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(5). out_cipher Err (RT_Enc_Hash_Data) en_integrity = true
    PRINTF("\r\n    (5). out_cipher Err test :    (en_integrity = true) \r\n");
    len_output = 32;
    len_cipherAes128 = sizeof(plainAes128) + 32 + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), NULL, &len_cipherAes128, true);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(6). out_cipher Err (RT_Enc_Hash_Data) en_integrity = false
    PRINTF("\r\n    (6). out_cipher Err test :    (en_integrity = false) \r\n");
    len_output = 32;
    len_cipherAes128 = sizeof(plainAes128) + 32 + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), NULL, &len_cipherAes128, false);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(7). out_length Err (RT_Enc_Hash_Data) en_integrity = true
    PRINTF("\r\n    (7). out_length Err test :    (en_integrity = true) \r\n");
    len_cipherAes128 = sizeof(plainAes128) + 32 + 15;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, true);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(8). out_length Err (RT_Enc_Hash_Data) en_integrity = false
    PRINTF("\r\n    (8). out_length Err test :    (en_integrity = false) \r\n");
    len_cipherAes128 = sizeof(plainAes128) + 15;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, false);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    PRINTF("\r\n    RT_Dec_Verify_Data(const uint8_t *in_cipher, uint32_t in_length, uint8_t  *out_plaintext, uint32_t *out_length, bool en_integrity) test :\r\n");

    //(1). in_cipher Err (RT_Dec_Verify_Data) en_integrity = true
    PRINTF("\r\n    (1). in_cipher Err test :    (en_integrity = true) \r\n");
    len_output = 32;
    len_cipherAes128 = sizeof(plainAes128) + 32 + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, true);
    if(kStatus_Success != ret_val) return ret_val;
    ret_val = RT_Dec_Verify_Data(NULL, sizeof(cipherAes128), output, &len_output, true);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(2). in_cipher Err (RT_Dec_Verify_Data) en_integrity = false
    PRINTF("\r\n    (2). in_cipher Err test :    (en_integrity = false) \r\n");
    len_output = 32;
    len_cipherAes128 = sizeof(plainAes128) + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, false);
    if(kStatus_Success != ret_val) return ret_val;
    ret_val = RT_Dec_Verify_Data(NULL, sizeof(cipherAes128), output, &len_output, false);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(3). in_length Err (RT_Dec_Verify_Data) en_integrity = true
    PRINTF("\r\n    (3). in_length Err test :    (en_integrity = true) \r\n");
    len_output = 32;
    len_cipherAes128 = sizeof(plainAes128) + 32 + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, true);
    if(kStatus_Success != ret_val) return ret_val;
    ret_val = RT_Dec_Verify_Data(cipherAes128, sizeof(cipherAes128) - 1, output, &len_output, true);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(4). in_length Err (RT_Dec_Verify_Data) en_integrity = false
    PRINTF("\r\n    (4). in_length Err test :    (en_integrity = false) \r\n");
    len_output = 32;
    len_cipherAes128 = sizeof(plainAes128) + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, false);
    if(kStatus_Success != ret_val) return ret_val;
    ret_val = RT_Dec_Verify_Data(cipherAes128, sizeof(cipherAes128) - 1, output, &len_output, false);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(5). out_plaintext Err (RT_Dec_Verify_Data) en_integrity = true
    PRINTF("\r\n    (5). out_plaintext Err test :    (en_integrity = true) \r\n");
    len_output = 32;
    len_cipherAes128 = sizeof(plainAes128) + 32 + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, true);
    if(kStatus_Success != ret_val) return ret_val;
    ret_val = RT_Dec_Verify_Data(cipherAes128, sizeof(cipherAes128) , NULL, &len_output, true);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(6). out_plaintext Err (RT_Dec_Verify_Data) en_integrity = false
    PRINTF("\r\n    (6). out_plaintext Err test :    (en_integrity = false) \r\n");
    len_output = 32;
    len_cipherAes128 = sizeof(plainAes128) + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, false);
    if(kStatus_Success != ret_val) return ret_val;
    ret_val = RT_Dec_Verify_Data(cipherAes128, sizeof(cipherAes128), NULL, &len_output, false);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(7). out_length Err (RT_Dec_Verify_Data) en_integrity = true
    PRINTF("\r\n    (7). out_length Err test :    (en_integrity = true) \r\n");
    len_output = 31;
    len_cipherAes128 = sizeof(plainAes128) + 32 + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, true);
    if(kStatus_Success != ret_val) return ret_val;
    ret_val = RT_Dec_Verify_Data(cipherAes128, sizeof(cipherAes128), output, &len_output, true);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

    //(8) out_length Err (RT_Dec_Verify_Data) en_integrity = false
    PRINTF("\r\n    (8). out_length Err test :    (en_integrity = false) \r\n");
    len_output = 31;
    len_cipherAes128 = sizeof(plainAes128) + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, false);
    if(kStatus_Success != ret_val) return ret_val;
    ret_val = RT_Dec_Verify_Data(cipherAes128, sizeof(cipherAes128), output, &len_output, false);
    if(kStatus_InvalidArgument == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

//  2. hash error
    PRINTF("\r\n2. Hash error test :\r\n");
    //hash error
    len_cipherAes128 = sizeof(plainAes128) + 32 + 16;
    len_output = 32;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, true);
    if(kStatus_Success != ret_val) return ret_val;

    memset(&cipherAes128[sizeof(plainAes128)], 0, 32);

    ret_val = RT_Dec_Verify_Data(cipherAes128, sizeof(cipherAes128), output, &len_output, true);
    if(kStatus_Fail == ret_val)
    {
        PRINTF("\r\n         Test result : SUCCESS!\r\n");
    }
    else
    {
        PRINTF("\r\n         Test result :  FAILED!\r\n");
        return kStatus_Fail;
    }

//  3. Parameters is OK
    PRINTF("\r\n3. Parameters is OK test :\r\n");
    PRINTF("\r\n    (1). en_integrity = true test : \r\n");
    len_output = 32;
    len_cipherAes128 = sizeof(plainAes128) + 32 + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, true);
    if(kStatus_Success != ret_val) return ret_val;
    ret_val = RT_Dec_Verify_Data(cipherAes128, sizeof(cipherAes128), output, &len_output, true);
    if(kStatus_Success != ret_val) return ret_val;

    if(memcmp(output, plainAes128, len_output) != 0)
    {
        PRINTF("\r\n         RT_Enc_Hash_Data and Enc_Dec_Verify_Data test failed!\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("\r\n         RT_Enc_Hash_Data and Enc_Dec_Verify_Data test pass!\r\n");
    }

    PRINTF("\r\n    (2). en_integrity = false test : \r\n");
    len_output = 32;
    len_cipherAes128 = sizeof(plainAes128) + 16;
    ret_val = RT_Enc_Hash_Data(plainAes128, sizeof(plainAes128), cipherAes128, &len_cipherAes128, false);
    if(kStatus_Success != ret_val) return ret_val;
    ret_val = RT_Dec_Verify_Data(cipherAes128, sizeof(cipherAes128) - 32, output, &len_output, false);  //32 = length of hash
    if(kStatus_Success != ret_val) return ret_val;

    if(memcmp(output, plainAes128, len_output) != 0)
    {
        PRINTF("\r\n         RT_Enc_Hash_Data and Enc_Dec_Verify_Data test failed!\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("\r\n         RT_Enc_Hash_Data and Enc_Dec_Verify_Data test pass!\r\n");
    }

    return ret_val;
}


