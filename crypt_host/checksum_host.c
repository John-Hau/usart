#include "checksum_host.h"

uint32_t get_crc32(uint8_t const * p_data, uint32_t size, uint32_t const * p_crc)
{
    uint32_t crc;

    crc = (p_crc == NULL) ? 0xFFFFFFFF : ~(*p_crc);
    for (uint32_t i = 0; i < size; i++)
    {
        crc = crc ^ p_data[i];
        for (uint32_t j = 8; j > 0; j--)
        {
            crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
        }
    }
    return ~crc;
}


uint32_t rt_checksum_crc(uint8_t const * p_data, uint32_t size)
{
    return get_crc32(p_data, size, NULL);
}


bool rt_checksum_crc_ok(uint8_t const * p_data, uint32_t size, uint32_t crc_chk)
{
    return (rt_checksum_crc(p_data, size) == crc_chk);
}


