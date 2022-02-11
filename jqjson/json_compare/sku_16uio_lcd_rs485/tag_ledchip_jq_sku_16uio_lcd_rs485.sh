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


#######################################################################
#tag ledchip 
########################################################################

echo ""
echo "#####################"
echo "tag ledchip"
echo "#####################"
echo ""

ledchip_0_enablePin_base=$(jq .${sku_name}.ledchip[0].enablePin.base ${json_file_name})
echo -n "ledchip_0_enablePin_base is ${ledchip_0_enablePin_base}"
if [[ "${ledchip_0_enablePin_base}" == "\"${GPIO1_BASE}\"" ]];then
	echo " which is GPIO_1"
else
	echo "ledchip_0_enablePin_base is error"
fi

ledchip_0_enablePin_pin=$(jq .${sku_name}.ledchip[0].enablePin.pin ${json_file_name})
echo -n "ledchip_0_enablePin_pin is ${ledchip_0_enablePin_pin}"
if [[ "${ledchip_0_enablePin_pin}" == "20" ]];then
	echo " which is 20"
else
	echo "ledchip_0_enablePin_pin is error"
fi

echo "NOTE:GPIO1_PIN20 is IOMUXC_GPIO_AD_B1_04_GPIO1_IO20 0x401F810CU, 0x5U, 0, 0, 0x401F82FCU "

ledchip_0_Slaveaddr=$(jq .${sku_name}.ledchip[0].Slaveaddr ${json_file_name})
echo -n "ledchip_0_Slaveaddr is ${ledchip_0_Slaveaddr}"
if [[ "${ledchip_0_Slaveaddr}" == "\"0x3C\"" ]];then
	echo " which is 0x3C"
else
	echo "ledchip_0_Slaveaddr is error"
fi


#----------------------------------------------------------------------------
echo ""
ledchip_1_enablePin_base=$(jq .${sku_name}.ledchip[1].enablePin.base ${json_file_name})
echo -n "ledchip_1_enablePin_base is ${ledchip_1_enablePin_base}"
if [[ "${ledchip_1_enablePin_base}" == "\"${GPIO3_BASE}\"" ]];then
	echo " which is GPIO_3"
else
	echo "ledchip_1_enablePin_base is error"
fi

ledchip_1_enablePin_pin=$(jq .${sku_name}.ledchip[1].enablePin.pin ${json_file_name})
echo -n "ledchip_1_enablePin_pin is ${ledchip_1_enablePin_pin}"
if [[ "${ledchip_1_enablePin_pin}" == "4" ]];then
	echo " which is 4"
else
	echo "ledchip_1_enablePin_pin is error"
fi


echo "NOTE:GPIO3_PIN4 is IOMUXC_GPIO_SD_B1_04_GPIO3_IO04 0x401F81E4U, 0x5U, 0, 0, 0x401F83D4U"

ledchip_1_Slaveaddr=$(jq .${sku_name}.ledchip[1].Slaveaddr ${json_file_name})
echo -n "ledchip_1_Slaveaddr is ${ledchip_1_Slaveaddr}"
if [[ "${ledchip_1_Slaveaddr}" == "\"0x3F\"" ]];then
	echo " which is 0x3F"
else
	echo "ledchip_1_Slaveaddr is error"
fi
