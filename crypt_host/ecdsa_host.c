/** \file
 *  \brief  ECDSA host interface for VSOM Platform
 *
 *  \date   November 2019
 *
 *  \copyright Honeywell
 *  ALL RIGHTS RESERVED, Honeywell Confidential and Proprietary.
 */

#include "ecdsa_host.h"
#include "ecc_host.h"

status_t rt_secp256r1_ecdsa_sign(void           const * p_private_key,
                                 uint8_t         const * p_hash,
                                 size_t          hash_size,
                                 uint8_t         * p_signature,
                                 size_t          signature_size)
{
    int mtls_ret;
    mbedtls_mpi       r_mpi;
    mbedtls_mpi       s_mpi;
    mbedtls_ecp_group group;
    
    rt_ecc_private_key_t *p_prv = (rt_ecc_private_key_t *)p_private_key;
    
    if((p_prv==NULL) || (p_hash==NULL) || (hash_size==0) ||
       (p_signature==NULL))
    {
        return kStatus_InvalidArgument;
    }
    if(signature_size != RT_ECC_SECP256R1_SIGNATURE_SIZE)
    {
        return kStatus_OutOfRange;
    }

    mbedtls_ecp_group_init(&group);    
    mtls_ret = mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1);
    if(mtls_ret)
    {
        return kStatus_Fail;
    }

    mbedtls_mpi_init(&r_mpi);
    mbedtls_mpi_init(&s_mpi);
    mtls_ret = mbedtls_ecdsa_sign(&group,
                                &r_mpi,
                                &s_mpi,
                                &p_prv->key,
                                p_hash,
                                hash_size,
                                rt_ecc_mbedtls_rng,
                                NULL);
    mbedtls_ecp_group_free(&group);
    
    if (mtls_ret == 0)
    {
        mtls_ret = mbedtls_mpi_write_binary(&r_mpi, p_signature, RT_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE);
        if (mtls_ret == 0)
        {
            mtls_ret = mbedtls_mpi_write_binary(&s_mpi,
                                              &p_signature[RT_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE],
                                              RT_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE);
        }
    }
    mbedtls_mpi_free(&r_mpi);
    mbedtls_mpi_free(&s_mpi);    
    
    if (mtls_ret != 0)
    {
        return kStatus_Fail;
    }
    return kStatus_Success;
}


status_t rt_secp256r1_ecdsa_verify(void       const * p_public_key,
                                    uint8_t    const * p_hash,
                                    size_t     hash_size,
                                    uint8_t    const * p_signature,
                                    size_t     signature_size)
{
    int mtls_ret;
    mbedtls_mpi       r_mpi;
    mbedtls_mpi       s_mpi;
    mbedtls_ecp_group group;
    
    rt_ecc_public_key_t *p_pub = (rt_ecc_public_key_t *)p_public_key;
    
    if((p_public_key==NULL) || (p_hash==NULL) || (hash_size==0) ||
       (p_signature==NULL))
    {
        return kStatus_InvalidArgument;
    }
    if(signature_size != RT_ECC_SECP256R1_SIGNATURE_SIZE)
    {
        return kStatus_OutOfRange;
    }

    mbedtls_ecp_group_init(&group);    
    mtls_ret = mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1);
    if(mtls_ret)
    {
        return kStatus_Fail;
    }

    mbedtls_mpi_init(&r_mpi);
    mbedtls_mpi_init(&s_mpi);
    mtls_ret = mbedtls_mpi_read_binary(&r_mpi, p_signature, RT_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE);
    if (mtls_ret == 0)
    {
        mtls_ret = mbedtls_mpi_read_binary(&s_mpi,
                                         &p_signature[RT_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE],
                                         RT_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE);
        if (mtls_ret == 0)
        {
            mtls_ret = mbedtls_ecdsa_verify(&group, p_hash, hash_size, &p_pub->key, &r_mpi, &s_mpi);
        }
    }

    mbedtls_ecp_group_free(&group);
    mbedtls_mpi_free(&r_mpi);
    mbedtls_mpi_free(&s_mpi);    

    if (mtls_ret == MBEDTLS_ERR_ECP_VERIFY_FAILED)
    {
        return kStatusInvalidSignature;
    }
    else if (mtls_ret != 0)
    {
        return kStatus_Fail;
    }
    return kStatus_Success;
}




status_t rt_ecdsa_test(void)
{
    status_t ret_val;
    
    rt_ecc_private_key_t priv_key;
    rt_ecc_public_key_t pub_key;
  
    ret_val = rt_secp256r1_gen_keypair(&priv_key, &pub_key);
    if(kStatus_Success != ret_val)
    {
        PRINTF("Gen keypair test fail!");
        return ret_val;
    }
    
    rt_secp256r1_signature_t m_signature;
    uint8_t m_hash[] =
    {
        0x42, 0xba, 0x83, 0x54, 0xdb, 0x26, 0x3a, 0x6a,
        0x5a, 0x9f, 0x74, 0xd6, 0xb7, 0xce, 0xb4, 0xc9,
        0x62, 0xa3, 0xd8, 0xfd, 0x58, 0xa4, 0x19, 0x69,
        0xe5, 0x21, 0xeb, 0x02, 0x22, 0x45, 0x54, 0x15,
    };

    ret_val = rt_secp256r1_ecdsa_sign(&priv_key,
                                     m_hash,
                                     sizeof(m_hash),
                                     m_signature,
                                     sizeof(m_signature));
    if(kStatus_Success != ret_val)
    {
        PRINTF("Sign fail!");
        return ret_val;
    }
    
    for(uint8_t i=0;i<32;i++)
    {
        PRINTF("0x%02x ", m_signature[i]);
    }
    PRINTF("\r\n");    
    
    ret_val = rt_secp256r1_ecdsa_verify(&pub_key,
                                       m_hash,
                                       sizeof(m_hash),
                                       m_signature,
                                       sizeof(m_signature));
    if(kStatus_Success != ret_val)
    {
        PRINTF("ECDSA verify fail!\r\n");
    }else{
        PRINTF("ECDSA verify pass!\r\n");
    }

    rt_secp256r1_private_key_free(&priv_key);
    rt_secp256r1_public_key_free(&pub_key);
    
    return kStatus_Success;
}


