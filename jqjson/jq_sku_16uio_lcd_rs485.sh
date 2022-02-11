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

if [[ "${i2c_0_base}" == "\"${LPI2C3_BASE}\"" || "${i2c_0_base}" == "\"${LPI2C3}\"" ]];then
	echo " which is LPI2C_3"
else
	echo "i2c_0_base is error"
fi




########################################################################
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


ledchip_1_Slaveaddr=$(jq .${sku_name}.ledchip[1].Slaveaddr ${json_file_name})
echo -n "ledchip_1_Slaveaddr is ${ledchip_1_Slaveaddr}"
if [[ "${ledchip_1_Slaveaddr}" == "\"0x3F\"" ]];then
	echo " which is 0x3F"
else
	echo "ledchip_1_Slaveaddr is error"
fi





########################################################################
#tag spi 
########################################################################

echo ""
echo "#####################"
echo "tag spi"
echo "#####################"
echo ""

spi_0_baud=$(jq .${sku_name}.spi[0].baud  ${json_file_name})
echo -n "spi_0_baud is ${spi_0_baud}"
if [[ "${spi_0_baud}" == "\"6000000\"" ]];then
	echo " which is 6000000"
else
	echo "spi_0_baud is error"
fi

spi_0_base=$(jq .${sku_name}.spi[0].base  ${json_file_name})
echo -n "spi_0_base is ${spi_0_base}"
if [[ "${spi_0_base}" == "\"${LPSPI4_BASE}\"" ]];then
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
if [[ "${spi_1_base}" == "\"${LPSPI3_BASE}\"" ]];then
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
















#i2c_0_base=`jq .sku_16uio_lcd_rs485.i2c[0].base tc_snap_on_io_dv2.json`
#echo -n "i2c_0_base is ${i2c_0_base}"

#if [[ "${i2c_0_base}" == "\"${LPI2C3_BASE}\"" || "${i2c_0_base}" == "\"${LPI2C3}\"" ]];then
#	echo " which is LPI2C_3"
#else
#	echo "i2c_0_base is error"
#fi



















