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
#tag rstpin 
########################################################################
echo ""
uiochip_rstpin_0_port=$(jq .${sku_name}.uiochip.rstpin[0].port  ${json_file_name})
echo -n "uiochip.rstpin_0_port is ${uiochip_rstpin_0_port}"
if [[ "${uiochip_rstpin_0_port}" == "\"${GPIO3}\"" || "${uiochip_rstpin_0_port}" == "\"${GPIO3_BASE}\"" ]];then
	echo " which is GPIO3"
else
	echo "uiochip_rstpin_0_port is error"
fi

uiochip_rstpin_0_pin=$(jq .${sku_name}.uiochip.rstpin[0].pin  ${json_file_name})
echo -n "uiochip.rstpin_0_pin is ${uiochip_rstpin_0_pin}"
if [[ "${uiochip_rstpin_0_pin}" == "\"17U\"" || "${uiochip_rstpin_0_pin}" == "\"17\"" ]];then
	echo " which is 17"
else
	echo "uiochip_rstpin_0_pin is error"
fi

echo ""
echo "GPIO3_PIN17 is IOMUXC_GPIO_SD_B0_05_GPIO3_IO17 0x401F81D0U, 0x5U, 0, 0, 0x401F83C0U"
echo ""

#----------------------------------------------------------------------
echo ""
uiochip_rstpin_1_port=$(jq .${sku_name}.uiochip.rstpin[1].port  ${json_file_name})
echo -n "uiochip.rstpin_1_port is ${uiochip_rstpin_1_port}"
if [[ "${uiochip_rstpin_1_port}" == "\"${GPIO2}\"" || "${uiochip_rstpin_1_port}" == "\"${GPIO2_BASE}\"" ]];then
	echo " which is GPIO2"
else
	echo "uiochip_rstpin_1_port is error"
fi

uiochip_rstpin_1_pin=$(jq .${sku_name}.uiochip.rstpin[1].pin  ${json_file_name})
echo -n "uiochip.rstpin_1_pin is ${uiochip_rstpin_1_pin}"
if [[ "${uiochip_rstpin_1_pin}" == "\"31U\"" || "${uiochip_rstpin_1_pin}" == "\"31\"" ]];then
	echo " which is 31"
else
	echo "uiochip_rstpin_1_pin is error"
fi


echo ""
echo "GPIO2_PIN31 IOMUXC_GPIO_B1_15_GPIO2_IO31 0x401F81B8U, 0x5U, 0, 0, 0x401F83A8U"
echo ""

#----------------------------------------------------------------------
echo ""
uiochip_rstpin_2_port=$(jq .${sku_name}.uiochip.rstpin[2].port  ${json_file_name})
echo -n "uiochip.rstpin_2_port is ${uiochip_rstpin_2_port}"
if [[ "${uiochip_rstpin_2_port}" == "\"${GPIO1}\"" || "${uiochip_rstpin_2_port}" == "\"${GPIO1_BASE}\"" ]];then
	echo " which is GPIO1"
else
	echo "uiochip_rstpin_2_port is error"
fi

uiochip_rstpin_2_pin=$(jq .${sku_name}.uiochip.rstpin[2].pin  ${json_file_name})
echo -n "uiochip.rstpin_2_pin is ${uiochip_rstpin_2_pin}"
if [[ "${uiochip_rstpin_2_pin}" == "\"11U\"" || "${uiochip_rstpin_2_pin}" == "\"11\"" ]];then
	echo " which is 11"
else
	echo "uiochip_rstpin_2_pin is error"
fi

echo ""
echo "GPIO1_PIN11 IOMUXC_GPIO_AD_B0_11_GPIO1_IO11 0x401F80E8U, 0x5U, 0, 0, 0x401F82D8U"
echo ""

#----------------------------------------------------------------------
echo ""
uiochip_rstpin_3_port=$(jq .${sku_name}.uiochip.rstpin[3].port  ${json_file_name})
echo -n "uiochip.rstpin_3_port is ${uiochip_rstpin_3_port}"
if [[ "${uiochip_rstpin_3_port}" == "\"${GPIO1}\"" || "${uiochip_rstpin_3_port}" == "\"${GPIO1_BASE}\"" ]];then
	echo " which is GPIO1"
else
	echo "uiochip_rstpin_3_port is error"
fi

uiochip_rstpin_3_pin=$(jq .${sku_name}.uiochip.rstpin[3].pin  ${json_file_name})
echo -n "uiochip.rstpin_3_pin is ${uiochip_rstpin_3_pin}"
if [[ "${uiochip_rstpin_3_pin}" == "\"24U\"" || "${uiochip_rstpin_3_pin}" == "\"24\"" ]];then
	echo " which is 24"
else
	echo "uiochip_rstpin_3_pin is error"
fi


echo ""
echo "GPIO1_PIN24 IOMUXC_GPIO_AD_B1_08_GPIO1_IO24 0x401F811CU, 0x5U, 0, 0, 0x401F830CU"
echo ""
