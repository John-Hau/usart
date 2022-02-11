#!/bin/bash

json_file_name="$1"
sku_name="$2"
  
#i2c_0_baud=$(jq .${sku_name}.i2c[0].baud  ${json_file_name})
#echo ${i2c_0_baud}
#exit

GPIO1_BASE="0x401B8000U"
GPIO2_BASE="0x401BC000U"
GPIO3_BASE="0x401C0000U"
GPIO4_BASE="0x401C4000U"
GPIO5_BASE="0x400C0000U"

GPIO1="0x401B8000u"
GPIO2="0x401BC000u"
GPIO3="0x401C0000u"
GPIO4="0x401C4000u"
GPIO5="0x400C0000u"

LPI2C1_BASE="0x403F0000U"
LPI2C2_BASE="0x403F4000U"
LPI2C3_BASE="0x403F8000U"
LPI2C4_BASE="0x403FC000U"

LPI2C1="0x403F0000"
LPI2C2="0x403F4000"
LPI2C3="0x403F8000"
LPI2C4="0x403FC000"

LPSPI1_BASE="0x40394000U"
LPSPI2_BASE="0x40398000U"
LPSPI3_BASE="0x4039C000U"
LPSPI4_BASE="0x403A0000U"

########################################################################
#tag i2c
########################################################################

echo ""
echo "#####################"
echo "tag i2c"
echo "#####################"
echo ""

i2c_0_baud=$(jq .${sku_name}.i2c[0].baud ${json_file_name})
echo "i2c_0_baud is ${i2c_0_baud}"


i2c_0_base=`jq .${sku_name}.i2c[0].base ${json_file_name}`
echo -n "i2c_0_base is ${i2c_0_base}"


#if [[ "${i2c_0_base}" == "\"${LPI2C3_BASE}\"" || "${i2c_0_base}" == "\"${LPI2C3}\"" ]];then
if [[ "${i2c_0_base}" == "\"${LPI2C3}\"" ]];then
         echo " which is LPI2C_3"
         echo "NOTE:LPI2C_3 is used for LED driver IS31FL3236A and EEPROM driver AT24C128"
else
         echo "i2c_0_base is error"
fi




