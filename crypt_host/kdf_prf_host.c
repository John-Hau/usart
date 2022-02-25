/** \file
 *  \brief  PRF host interface for VSOM Platform
 *
 *  \date   November 2019
 *
 *  \copyright Honeywell
 *  ALL RIGHTS RESERVED, Honeywell Confidential and Proprietary.
 */

#include "kdf_prf_host.h"

status_t rt_prf_calculate(uint8_t                        * const p_output_key,
                          size_t                         * const p_output_key_size,
                          uint8_t                  const * const p_input_key,
                          size_t                                 input_key_size,
                          uint8_t                  const * const p_ainfo,
                          size_t                                 ainfo_size)
{
    size_t nb;
    size_t dlen = *p_output_key_size;
    size_t i, j, k, md_len;
    const mbedtls_md_info_t *md_info = &mbedtls_sha256_info;
    mbedtls_md_context_t md_ctx;
    unsigned char tmp[128];
    unsigned char h_i[MBEDTLS_MD_MAX_SIZE];
    mbedtls_sha256_context ctx;
    unsigned char hamc_ctx[128];
    
    mbedtls_sha256_init( &ctx );
    mbedtls_md_init( &md_ctx );
    md_ctx.md_info = md_info;
    md_ctx.md_ctx = &ctx;
    md_ctx.hmac_ctx = hamc_ctx;
    
    md_len = mbedtls_md_get_size( md_info );
    memcpy( tmp + md_len, p_ainfo, ainfo_size );
    nb = ainfo_size;
    
    *p_output_key_size = 0; // Set output length to 0 as default value (in case of error).
    
    mbedtls_md_hmac_starts( &md_ctx, p_input_key, input_key_size );
    mbedtls_md_hmac_update( &md_ctx, tmp + md_len, nb );
    mbedtls_md_hmac_finish( &md_ctx, tmp );

    for( i = 0; i < dlen; i += md_len )
    {
        mbedtls_md_hmac_reset ( &md_ctx );
        mbedtls_md_hmac_update( &md_ctx, tmp, md_len + nb );
        mbedtls_md_hmac_finish( &md_ctx, h_i );

        mbedtls_md_hmac_reset ( &md_ctx );
        mbedtls_md_hmac_update( &md_ctx, tmp, md_len );
        mbedtls_md_hmac_finish( &md_ctx, tmp );

        k = ( i + md_len > dlen ) ? dlen % md_len : md_len;

        for( j = 0; j < k; j++ )
            p_output_key[i + j]  = h_i[j];
    }
    
    mbedtls_platform_zeroize( tmp, sizeof( tmp ) );
    mbedtls_platform_zeroize( h_i, sizeof( h_i ) );
    
    *p_output_key_size = dlen;
  
    return kStatus_Success;
}


status_t rt_load_ota_key_host(uint8_t* root_key, uint8_t* out_key, uint8_t* params, uint8_t params_len)
{
    status_t ret_code;
    uint8_t key_mid[32];
    uint8_t i;
    size_t okm_len;

    okm_len = sizeof(key_mid);
    ret_code = rt_prf_calculate(key_mid,
                                   &okm_len,
                                   root_key,
                                   32,
                                   params,
                                   params_len);
    
    // XOR
    for(i=0;i<16;i++){
        out_key[i] = key_mid[i] ^ key_mid[i + 16];
    }
    
    memset(key_mid, 0, sizeof(key_mid));

    return ret_code;
}


