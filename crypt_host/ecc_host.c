/** \file
 *  \brief  ECC host interface for VSOM Platform
 *
 *  \date   November 2019
 *
 *  \copyright Honeywell
 *  ALL RIGHTS RESERVED, Honeywell Confidential and Proprietary.
 */

#include "ecc_host.h"
#include "rng_host.h"

int rt_ecc_mbedtls_rng(void * p_param, unsigned char * p_data, size_t size)
{
    status_t ret_val;
  
    ret_val = rt_rng_vector_generate(p_data, size);
    if(kStatus_Success != ret_val)
    {
        return MBEDTLS_ERR_ECP_RANDOM_FAILED;
    }

    return 0;
}


status_t rt_secp256r1_gen_keypair(void * p_private_key,
                                   void * p_public_key)
{
    int mtls_ret;
    mbedtls_ecp_group group;
    
    rt_ecc_private_key_t *p_prv = (rt_ecc_private_key_t *)p_private_key;
    rt_ecc_public_key_t *p_pub = (rt_ecc_public_key_t *)p_public_key;
    
    if((p_prv==NULL) || (p_pub==NULL))
    {
        return kStatus_InvalidArgument;
    }
    
    mbedtls_ecp_group_init(&group);
    mbedtls_mpi_init(&p_prv->key);
    mbedtls_ecp_point_init(&p_pub->key);
    
    mtls_ret = mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1);
    if(mtls_ret)
    {
        return kStatus_Fail;
    }
      
    mtls_ret = mbedtls_ecp_gen_keypair( &group, 
                                       &p_prv->key, 
                                       &p_pub->key, 
                                       rt_ecc_mbedtls_rng, 
                                       NULL );
    mbedtls_ecp_group_free(&group);
    if(mtls_ret)
    {
        mbedtls_mpi_free(&p_prv->key);
        mbedtls_ecp_point_free(&p_pub->key);
        return kStatus_Fail;
    }

    return kStatus_Success;
}


status_t rt_secp256r1_private_key_free(void * p_private_key)
{       
    rt_ecc_private_key_t *p_prv = (rt_ecc_private_key_t *)p_private_key;
    
    mbedtls_mpi_free(&p_prv->key);
    return kStatus_Success;
}


status_t rt_secp256r1_public_key_free(void * p_public_key)
{   
    rt_ecc_public_key_t *p_pub = (rt_ecc_public_key_t *)p_public_key;

    mbedtls_ecp_point_free(&p_pub->key);
    return kStatus_Success;
}



status_t rt_secp256r1_public_key_from_raw(void * p_public_key, 
                                          uint8_t const * p_raw_data,
                                          size_t raw_data_size)
{   
    int mtls_ret;
    rt_ecc_public_key_t *p_pub = (rt_ecc_public_key_t *)p_public_key;
    mbedtls_ecp_group group;
    size_t plen;

    if((p_public_key==NULL) || (p_raw_data==NULL))
    {
        return kStatus_InvalidArgument;
    }
    if(raw_data_size != RT_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE)
    {
        return kStatus_OutOfRange;
    }

    mbedtls_ecp_group_init(&group);
    mbedtls_ecp_point_init(&p_pub->key);
    mtls_ret = mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1);
    if(mtls_ret)
    {
        return kStatus_Fail;
    }    
    
    plen = mbedtls_mpi_size( &group.P );
    
    mtls_ret = mbedtls_mpi_read_binary(&p_pub->key.X,
                                       p_raw_data,
                                       plen);
    if (mtls_ret != 0)
    {
        goto error_exit;
    }
    
    mtls_ret = mbedtls_mpi_read_binary(&p_pub->key.Y,
                                         &p_raw_data[plen],
                                         plen);
    if (mtls_ret != 0)
    {
        goto error_exit;
    }

    mtls_ret = mbedtls_mpi_lset(&p_pub->key.Z, 1);
    if (mtls_ret == 0)
    {
        mtls_ret = mbedtls_ecp_check_pubkey(&group, &p_pub->key);
    }
    
    if (mtls_ret != 0)
    {
        goto error_exit;
    }
    
    mbedtls_ecp_group_free(&group);
    return kStatus_Success;

error_exit:
    mbedtls_ecp_group_free(&group);
    mbedtls_ecp_point_free(&p_pub->key);
    
    return kStatusInvalidInternal;    
}




