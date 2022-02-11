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
#tag knob 
########################################################################
echo ""
knob_cfg_0_port=$(jq .${sku_name}.knob.cfg[0].port  ${json_file_name})
echo -n "knob_cfg_0_port is ${knob_cfg_0_port}"
if [[ "${knob_cfg_0_port}" == "\"${GPIO3}\"" || "${knob_cfg_0_port}" == "\"${GPIO3_BASE}\"" ]];then
	echo " which is GPIO3"
else
	echo "knob_cfg_0_port is error"
fi

knob_cfg_0_pin=$(jq .${sku_name}.knob.cfg[0].pin  ${json_file_name})
echo -n "knob_cfg_0_pin is ${knob_cfg_0_pin}"
if [[ "${knob_cfg_0_pin}" == "4" || "${knob_cfg_0_pin}" == "4U" ]];then
	echo " which is 4"
else
	echo "knob_cfg_0_pin is error"
fi

echo ""
echo "GPIO3_PIN4 is IOMUXC_GPIO_SD_B1_04_GPIO3_IO04 0x401F81E4U, 0x5U, 0, 0, 0x401F83D4U"
echo ""

#------------------------------------------------------------

echo ""
knob_cfg_1_port=$(jq .${sku_name}.knob.cfg[1].port  ${json_file_name})
echo -n "knob_cfg_1_port is ${knob_cfg_1_port}"
if [[ "${knob_cfg_1_port}" == "\"${GPIO3}\"" || "${knob_cfg_1_port}" == "\"${GPIO3_BASE}\"" ]];then
	echo " which is GPIO3"
else
	echo "knob_cfg_1_port is error"
fi

knob_cfg_1_pin=$(jq .${sku_name}.knob.cfg[1].pin  ${json_file_name})
echo -n "knob_cfg_1_pin is ${knob_cfg_1_pin}"
if [[ "${knob_cfg_1_pin}" == "2" || "${knob_cfg_1_pin}" == "2U" ]];then
	echo " which is 2"
else
	echo "knob_cfg_1_pin is error"
fi


echo ""
echo "GPIO3_PIN2 is IOMUXC_GPIO_SD_B1_02_GPIO3_IO02 0x401F81DCU, 0x5U, 0, 0, 0x401F83CCU"
echo ""

#------------------------------------------------------------

echo ""
knob_cfg_2_port=$(jq .${sku_name}.knob.cfg[2].port  ${json_file_name})
echo -n "knob_cfg_2_port is ${knob_cfg_2_port}"
if [[ "${knob_cfg_2_port}" == "\"${GPIO3}\"" || "${knob_cfg_2_port}" == "\"${GPIO3_BASE}\"" ]];then
	echo " which is GPIO3"
else
	echo "knob_cfg_2_port is error"
fi

knob_cfg_2_pin=$(jq .${sku_name}.knob.cfg[2].pin  ${json_file_name})
echo -n "knob_cfg_2_pin is ${knob_cfg_2_pin}"
if [[ "${knob_cfg_2_pin}" == "5" || "${knob_cfg_2_pin}" == "5U" ]];then
	echo " which is 5"
else
	echo "knob_cfg_2_pin is error"
fi

echo ""
echo "GPIO3_PIN5 is IOMUXC_GPIO_SD_B1_05_GPIO3_IO05 0x401F81E8U, 0x5U, 0, 0, 0x401F83D8U"
echo ""


#----------------------------------------------------------

echo ""
knob_cfg_3_port=$(jq .${sku_name}.knob.cfg[3].port  ${json_file_name})
echo -n "knob_cfg_3_port is ${knob_cfg_3_port}"
if [[ "${knob_cfg_3_port}" == "\"${GPIO5}\"" || "${knob_cfg_3_port}" == "\"${GPIO5_BASE}\"" ]];then
	echo " which is GPIO5"
else
	echo "knob_cfg_3_port is error"
fi

knob_cfg_3_pin=$(jq .${sku_name}.knob.cfg[3].pin  ${json_file_name})
echo -n "knob_cfg_3_pin is ${knob_cfg_3_pin}"
if [[ "${knob_cfg_3_pin}" == "0" || "${knob_cfg_3_pin}" == "0U" ]];then
	echo " which is 0"
else
	echo "knob_cfg_3_pin is error"
fi


echo ""
echo "GPIO5_PIN0 is IOMUXC_SNVS_WAKEUP_GPIO5_IO00 0x400A8000U, 0x5U, 0, 0, 0x400A8018U"
echo ""
