/**
 * @file kdf.c
 * @author Michal Hojsik (michal.hojsik@honeywell.com)
 * @brief KDF (Key Derivation Function) implementation
 * @date 2020-05-18
 *
 * This module implements Key Derivation Function in counter mode 
 * using CMAC as pseudorandom function as defined in 
 * NIST Special Publication 800-108, October 2009.
 *
 * The secret key used for the key derivation is the device unique key.
 * 
 * There is no key input to the KDF function. The KDF calls CMAC module
 * to perform all key-dependent computations.
 * 
 * @copyright Copyright 2020 Honeywell International Inc. All rights reserved.
 */
/*
 * THIS DOCUMENT CONTAINS PROPRIETARY INFORMATION OF HONEYWELL INTERNATIONAL INC.
 * NEITHER THIS DOCUMENT NOR THE INFORMATION CONTAINED HEREIN MAY BE REPRODUCED, USED,
 * DISTRIBUTED OR DISCLOSED TO OTHERS WITHOUT THE WRITTEN CONSENT OF HONEYWELL.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h> /* For memcpy */
#include <assert.h>
#include "cmac1.h"
#include "kdf.h"

#define KDF_BUFFER_SIZE (256U)

mcrypto_status_t mcrypto_kdf_with_device_key(uint8_t* key, uint32_t key_len, const unsigned char* label, uint32_t label_len, const unsigned char* context, uint32_t context_len)
{
    /* This function implements Key Derivation Function in counter mode.
     * It generates a secret key of a size 16 to 4096 bytes.
     * 
     * Input data to the PRF as per NIST SP 800-108 consist of
     * index - 1 byte - index of the PRF call - each call gives 16 bytes of the generate key so we can generate up to 16*256 = 4096 bytes of key material
     * Label - input to the function
     * 0x00 - 1 zero byte
     * Context - input to the function
     * [key bit length] - Bit length of the derived key - 4 byte value
     */ 

    if ((NULL == key) || (key_len < 16) || (key_len > 4096) || (NULL == label) || (0 == label_len) || (NULL == context) || (0 == context_len))
    {
        return MCRYPTO_INVALID_ARGS;
    }

    /* Verify that the input label and context will fit into out KDF buffer together with the fixed inputs - index (1B), 0x00 (1B), key bit length (4B).*/
    if ( (KDF_BUFFER_SIZE - 6) < (label_len + context_len) )
    {
        return MCRYPTO_INVALID_ARGS;
    }

    mcrypto_status_t status;

    /* buffer to create input to the PRF */
    uint8_t buffer[KDF_BUFFER_SIZE];
    uint32_t buffer_index;
    uint32_t key_index = 0;
    uint32_t len;

    mcrypto_auth_tag_t tag;

    /* Prepare the PRF input of the form
     * index | label | 0x00 | context | [key bit-length]
     */
    buffer_index = 0;

    /* The first byte of the buffer is set the index of the PRF call */
    /* Start with 0 and increase it after every PRF call */
    buffer[buffer_index] = 0;
    buffer_index++;

    /* Copy the label */
    memcpy(buffer + buffer_index, (void*) label, label_len);
    buffer_index += label_len;
    
    /* Zero byte between the label and the context */
    buffer[buffer_index] = 0x00;
    buffer_index++;
    
    /* Copy the context */
    memcpy(buffer + buffer_index, (void*) context, context_len);
    buffer_index += context_len;
    
    /* Generated key bit-length as 4-byte integer */
    *(uint32_t*)(buffer + buffer_index) = key_len*8;
    buffer_index += 4;
    
    /* Generate the key in 16-byte chunks */
    while (key_index < key_len)
    {
        /* Call CMAC to compute the resulting key */
        status = mcrypto_aes_cmac_with_devkey(buffer, buffer_index, &tag);
        if (MCRYPTO_OK != status)
        {
            return status;
        }
    
        len = (key_index + MCRYPTO_AUTH_TAG_SIZE > key_len) ? key_len - key_index : MCRYPTO_AUTH_TAG_SIZE;
        memcpy(&(key[key_index]), tag.data, len);
        
        key_index += len;
        buffer[0]++;
    }

    return MCRYPTO_OK;
}
