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

LPSPI1="0x40394000u"
LPSPI2="0x40398000u"
LPSPI3="0x4039C000u"
LPSPI4="0x403A0000u"


########################################################################
#tag spi 
########################################################################

echo ""
echo "#####################"
echo "tag spi"
echo "#####################"
echo ""
echo "NOTE:LPSPI3 for LCD driver and LPSPI4 for UIO driver"
echo ""
spi_0_baud=$(jq .${sku_name}.spi[0].baud  ${json_file_name})
echo -n "spi_0_baud is ${spi_0_baud}"
if [[ "${spi_0_baud}" == "\"3000000\"" ]];then
	echo " which is 3000000"
else
	echo "spi_0_baud is error"
fi

spi_0_base=$(jq .${sku_name}.spi[0].base  ${json_file_name})
echo -n "spi_0_base is ${spi_0_base}"
if [[ "${spi_0_base}" == "\"${LPSPI4_BASE}\"" || "${spi_0_base}" == "\"${LPSPI4}\"" ]];then
	echo " which is LPSPI4"
else
	echo "spi_0_base is error"
fi


spi_0_cpol=$(jq .${sku_name}.spi[0].cpol  ${json_file_name})
echo -n "spi_0_cpol is ${spi_0_cpol}"
if [[ "${spi_0_cpol}" == "\"0\"" ]];then
	echo " which is 0"
else
	echo "spi_0_cpol is error"
fi


spi_0_cphase=$(jq .${sku_name}.spi[0].cphase  ${json_file_name})
echo -n "spi_0_cphase is ${spi_0_cphase}"
if [[ "${spi_0_cphase}" == "\"1\"" ]];then
	echo " which is 1"
else
	echo "spi_0_cphase is error"
fi

#-------------------------------------------------------------
echo ""
spi_1_baud=$(jq .${sku_name}.spi[1].baud  ${json_file_name})
echo -n "spi_1_baud is ${spi_1_baud}"
if [[ "${spi_1_baud}" == "\"5000000\"" ]];then
	echo " which is 5000000"
else
	echo "spi_1_baud is error"
fi

spi_1_base=$(jq .${sku_name}.spi[1].base  ${json_file_name})
echo -n "spi_1_base is ${spi_1_base}"
if [[ "${spi_1_base}" == "\"${LPSPI3_BASE}\"" || "${spi_1_base}" == "\"${LPSPI3}\"" ]];then
	echo " which is LPSPI3"
else
	echo "spi_1_base is error"
fi


spi_1_cpol=$(jq .${sku_name}.spi[1].cpol  ${json_file_name})
echo -n "spi_1_cpol is ${spi_1_cpol}"
if [[ "${spi_1_cpol}" == "\"1\"" ]];then
	echo " which is 1"
else
	echo "spi_1_cpol is error"
fi


spi_1_cphase=$(jq .${sku_name}.spi[1].cphase  ${json_file_name})
echo -n "spi_1_cphase is ${spi_1_cphase}"
if [[ "${spi_1_cphase}" == "\"1\"" ]];then
	echo " which is 1"
else
	echo "spi_1_cphase is error"
fi

