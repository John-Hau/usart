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

#0x403F8000=LPI2C3
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
if [[ "${spi_0_baud}" == "\"3000000\"" ]];then
	echo " which is 3000000"
else
	echo "spi_0_baud is error"
fi

#0x403A0000=LPSPI4
spi_0_base=$(jq .${sku_name}.spi[0].base  ${json_file_name})
echo -n "spi_0_base is ${spi_0_base}"
if [[ "${spi_0_base}" == "\"${LPSPI4_BASE}\"" ]];then
	echo " which is LPSPI4"
else
	echo "spi_0_base is error"
fi


spi_0_cpol=$(jq .${sku_name}.spi[0].cpol  ${json_file_name})
echo -n "spi_0_cpol is ${spi_0_cpol}"
if [[ "${spi_0_cpol}" == "\"1\"" ]];then
	echo " which is 1"
else
	echo "spi_0_cpol is error"
fi


spi_0_cphase=$(jq .${sku_name}.spi[0].cphase  ${json_file_name})
echo -n "spi_0_cphase is ${spi_0_cphase}"
if [[ "${spi_0_cphase}" == "\"0\"" ]];then
	echo " which is 0"
else
	echo "spi_0_cphase is error"
fi

#-------------------------------------------------------------






########################################################################
#tag bi 
########################################################################
echo ""

bi_chcfg_0_chIndex=$(jq .${sku_name}.bi.chcfg[0].chIndex  ${json_file_name})
echo -n "bi_chcfg_0_chIndex is ${bi_chcfg_0_chIndex}"
if [[ "${bi_chcfg_0_chIndex}" == "0" ]];then
	echo " which is 0"
else
	echo "bi_chcfg_0_chIndex is error"
fi


bi_chcfg_0_port=$(jq .${sku_name}.bi.chcfg[0].port  ${json_file_name})
echo -n "bi_chcfg_0_port is ${bi_chcfg_0_port}"
if [[ "${bi_chcfg_0_port}" == "\"${GPIO2}\"" || "${bi_chcfg_0_port}" == "\"${GPIO2_BASE}\"" ]];then
	echo " which is GPIO2"
else
	echo "bi_chcfg_0_port is error"
fi

bi_chcfg_0_pin=$(jq .${sku_name}.bi.chcfg[0].pin  ${json_file_name})
echo -n "bi_chcfg_0_pin is ${bi_chcfg_0_pin}"
if [[ "${bi_chcfg_0_pin}" == "25" || "${bi_chcfg_0_pin}" == "25U" ]];then
	echo " which is 25"
else
	echo "bi_chcfg_0_pin is error"
fi



#----------------------------------------------------------
echo ""

bi_chcfg_1_chIndex=$(jq .${sku_name}.bi.chcfg[1].chIndex  ${json_file_name})
echo -n "bi_chcfg_1_chIndex is ${bi_chcfg_1_chIndex}"
if [[ "${bi_chcfg_1_chIndex}" == "1" ]];then
	echo " which is 1"
else
	echo "bi_chcfg_1_chIndex is error"
fi


bi_chcfg_1_port=$(jq .${sku_name}.bi.chcfg[1].port  ${json_file_name})
echo -n "bi_chcfg_1_port is ${bi_chcfg_1_port}"
if [[ "${bi_chcfg_1_port}" == "\"${GPIO2}\"" || "${bi_chcfg_1_port}" == "\"${GPIO2_BASE}\"" ]];then
	echo " which is GPIO2"
else
	echo "bi_chcfg_1_port is error"
fi

bi_chcfg_1_pin=$(jq .${sku_name}.bi.chcfg[1].pin  ${json_file_name})
echo -n "bi_chcfg_1_pin is ${bi_chcfg_1_pin}"
if [[ "${bi_chcfg_1_pin}" == "24" || "${bi_chcfg_1_pin}" == "24U" ]];then
	echo " which is 24"
else
	echo "bi_chcfg_1_pin is error"
fi






#----------------------------------------------------------
echo ""

bi_chcfg_2_chIndex=$(jq .${sku_name}.bi.chcfg[2].chIndex  ${json_file_name})
echo -n "bi_chcfg_2_chIndex is ${bi_chcfg_2_chIndex}"
if [[ "${bi_chcfg_2_chIndex}" == "2" ]];then
	echo " which is 2"
else
	echo "bi_chcfg_2_chIndex is error"
fi


bi_chcfg_2_port=$(jq .${sku_name}.bi.chcfg[2].port  ${json_file_name})
echo -n "bi_chcfg_2_port is ${bi_chcfg_2_port}"
if [[ "${bi_chcfg_2_port}" == "\"${GPIO3}\"" || "${bi_chcfg_2_port}" == "\"${GPIO3_BASE}\"" ]];then
	echo " which is GPIO3"
else
	echo "bi_chcfg_2_port is error"
fi

bi_chcfg_2_pin=$(jq .${sku_name}.bi.chcfg[2].pin  ${json_file_name})
echo -n "bi_chcfg_2_pin is ${bi_chcfg_2_pin}"
if [[ "${bi_chcfg_2_pin}" == "16" || "${bi_chcfg_2_pin}" == "16U" ]];then
	echo " which is 16"
else
	echo "bi_chcfg_2_pin is error"
fi



#----------------------------------------------------------
echo ""

bi_chcfg_3_chIndex=$(jq .${sku_name}.bi.chcfg[3].chIndex  ${json_file_name})
echo -n "bi_chcfg_3_chIndex is ${bi_chcfg_3_chIndex}"
if [[ "${bi_chcfg_3_chIndex}" == "3" ]];then
	echo " which is 3"
else
	echo "bi_chcfg_3_chIndex is error"
fi


bi_chcfg_3_port=$(jq .${sku_name}.bi.chcfg[3].port  ${json_file_name})
echo -n "bi_chcfg_3_port is ${bi_chcfg_3_port}"
if [[ "${bi_chcfg_2_port}" == "\"${GPIO2}\"" || "${bi_chcfg_2_port}" == "\"${GPIO2_BASE}\"" ]];then
	echo " which is GPIO2"
else
	echo "bi_chcfg_3_port is error"
fi

bi_chcfg_3_pin=$(jq .${sku_name}.bi.chcfg[3].pin  ${json_file_name})
echo -n "bi_chcfg_3_pin is ${bi_chcfg_3_pin}"
if [[ "${bi_chcfg_3_pin}" == "30" || "${bi_chcfg_3_pin}" == "30U" ]];then
	echo " which is 30"
else
	echo "bi_chcfg_3_pin is error"
fi






#----------------------------------------------------------
echo ""

bi_chcfg_4_chIndex=$(jq .${sku_name}.bi.chcfg[4].chIndex  ${json_file_name})
echo -n "bi_chcfg_4_chIndex is ${bi_chcfg_4_chIndex}"
if [[ "${bi_chcfg_4_chIndex}" == "4" ]];then
	echo " which is 4"
else
	echo "bi_chcfg_4_chIndex is error"
fi


bi_chcfg_4_port=$(jq .${sku_name}.bi.chcfg[4].port  ${json_file_name})
echo -n "bi_chcfg_4_port is ${bi_chcfg_4_port}"
if [[ "${bi_chcfg_4_port}" == "\"${GPIO3}\"" || "${bi_chcfg_4_port}" == "\"${GPIO3_BASE}\"" ]];then
	echo " which is GPIO3"
else
	echo "bi_chcfg_4_port is error"
fi

bi_chcfg_4_pin=$(jq .${sku_name}.bi.chcfg[4].pin  ${json_file_name})
echo -n "bi_chcfg_4_pin is ${bi_chcfg_4_pin}"
if [[ "${bi_chcfg_4_pin}" == "17" || "${bi_chcfg_4_pin}" == "17U" ]];then
	echo " which is 17"
else
	echo "bi_chcfg_4_pin is error"
fi



#----------------------------------------------------------
echo ""

bi_chcfg_5_chIndex=$(jq .${sku_name}.bi.chcfg[5].chIndex  ${json_file_name})
echo -n "bi_chcfg_5_chIndex is ${bi_chcfg_5_chIndex}"
if [[ "${bi_chcfg_5_chIndex}" == "5" ]];then
	echo " which is 5"
else
	echo "bi_chcfg_5_chIndex is error"
fi


bi_chcfg_5_port=$(jq .${sku_name}.bi.chcfg[5].port  ${json_file_name})
echo -n "bi_chcfg_5_port is ${bi_chcfg_5_port}"
if [[ "${bi_chcfg_5_port}" == "\"${GPIO2}\"" || "${bi_chcfg_5_port}" == "\"${GPIO2_BASE}\"" ]];then
	echo " which is GPIO2"
else
	echo "bi_chcfg_5_port is error"
fi

bi_chcfg_5_pin=$(jq .${sku_name}.bi.chcfg[5].pin  ${json_file_name})
echo -n "bi_chcfg_5_pin is ${bi_chcfg_5_pin}"
if [[ "${bi_chcfg_5_pin}" == "31" || "${bi_chcfg_5_pin}" == "31U" ]];then
	echo " which is 31"
else
	echo "bi_chcfg_5_pin is error"
fi



#----------------------------------------------------------
echo ""

bi_chcfg_6_chIndex=$(jq .${sku_name}.bi.chcfg[6].chIndex  ${json_file_name})
echo -n "bi_chcfg_6_chIndex is ${bi_chcfg_6_chIndex}"
if [[ "${bi_chcfg_6_chIndex}" == "6" ]];then
	echo " which is 6"
else
	echo "bi_chcfg_6_chIndex is error"
fi


bi_chcfg_6_port=$(jq .${sku_name}.bi.chcfg[6].port  ${json_file_name})
echo -n "bi_chcfg_6_port is ${bi_chcfg_6_port}"
if [[ "${bi_chcfg_6_port}" == "\"${GPIO1}\"" || "${bi_chcfg_6_port}" == "\"${GPIO1_BASE}\"" ]];then
	echo " which is GPIO1"
else
	echo "bi_chcfg_6_port is error"
fi

bi_chcfg_6_pin=$(jq .${sku_name}.bi.chcfg[6].pin  ${json_file_name})
echo -n "bi_chcfg_6_pin is ${bi_chcfg_6_pin}"
if [[ "${bi_chcfg_6_pin}" == "11" || "${bi_chcfg_6_pin}" == "11U" ]];then
	echo " which is 11"
else
	echo "bi_chcfg_6_pin is error"
fi





#----------------------------------------------------------
echo ""

bi_chcfg_7_chIndex=$(jq .${sku_name}.bi.chcfg[7].chIndex  ${json_file_name})
echo -n "bi_chcfg_7_chIndex is ${bi_chcfg_7_chIndex}"
if [[ "${bi_chcfg_7_chIndex}" == "7" ]];then
	echo " which is 7"
else
	echo "bi_chcfg_7_chIndex is error"
fi


bi_chcfg_7_port=$(jq .${sku_name}.bi.chcfg[7].port  ${json_file_name})
echo -n "bi_chcfg_7_port is ${bi_chcfg_7_port}"
if [[ "${bi_chcfg_7_port}" == "\"${GPIO1}\"" || "${bi_chcfg_7_port}" == "\"${GPIO1_BASE}\"" ]];then
	echo " which is GPIO1"
else
	echo "bi_chcfg_7_port is error"
fi

bi_chcfg_7_pin=$(jq .${sku_name}.bi.chcfg[7].pin  ${json_file_name})
echo -n "bi_chcfg_7_pin is ${bi_chcfg_7_pin}"
if [[ "${bi_chcfg_7_pin}" == "24" || "${bi_chcfg_7_pin}" == "24U" ]];then
	echo " which is 24"
else
	echo "bi_chcfg_7_pin is error"
fi






#----------------------------------------------------------
echo ""

bi_chcfg_8_chIndex=$(jq .${sku_name}.bi.chcfg[8].chIndex  ${json_file_name})
echo -n "bi_chcfg_8_chIndex is ${bi_chcfg_8_chIndex}"
if [[ "${bi_chcfg_8_chIndex}" == "8" ]];then
	echo " which is 8"
else
	echo "bi_chcfg_8_chIndex is error"
fi


bi_chcfg_8_port=$(jq .${sku_name}.bi.chcfg[8].port  ${json_file_name})
echo -n "bi_chcfg_8_port is ${bi_chcfg_8_port}"
if [[ "${bi_chcfg_8_port}" == "\"${GPIO1}\"" || "${bi_chcfg_8_port}" == "\"${GPIO1_BASE}\"" ]];then
	echo " which is GPIO1"
else
	echo "bi_chcfg_8_port is error"
fi

bi_chcfg_8_pin=$(jq .${sku_name}.bi.chcfg[8].pin  ${json_file_name})
echo -n "bi_chcfg_8_pin is ${bi_chcfg_8_pin}"
if [[ "${bi_chcfg_8_pin}" == "14" || "${bi_chcfg_8_pin}" == "14U" ]];then
	echo " which is 14"
else
	echo "bi_chcfg_8_pin is error"
fi




#----------------------------------------------------------
echo ""

bi_chcfg_9_chIndex=$(jq .${sku_name}.bi.chcfg[9].chIndex  ${json_file_name})
echo -n "bi_chcfg_9_chIndex is ${bi_chcfg_9_chIndex}"
if [[ "${bi_chcfg_9_chIndex}" == "9" ]];then
	echo " which is 9"
else
	echo "bi_chcfg_9_chIndex is error"
fi


bi_chcfg_9_port=$(jq .${sku_name}.bi.chcfg[9].port  ${json_file_name})
echo -n "bi_chcfg_9_port is ${bi_chcfg_9_port}"
if [[ "${bi_chcfg_9_port}" == "\"${GPIO3}\"" || "${bi_chcfg_9_port}" == "\"${GPIO3_BASE}\"" ]];then
	echo " which is GPIO3"
else
	echo "bi_chcfg_9_port is error"
fi

bi_chcfg_9_pin=$(jq .${sku_name}.bi.chcfg[9].pin  ${json_file_name})
echo -n "bi_chcfg_9_pin is ${bi_chcfg_9_pin}"
if [[ "${bi_chcfg_9_pin}" == "26" || "${bi_chcfg_9_pin}" == "26U" ]];then
	echo " which is 26"
else
	echo "bi_chcfg_9_pin is error"
fi




#----------------------------------------------------------
echo ""

bi_chcfg_10_chIndex=$(jq .${sku_name}.bi.chcfg[10].chIndex  ${json_file_name})
echo -n "bi_chcfg_10_chIndex is ${bi_chcfg_10_chIndex}"
if [[ "${bi_chcfg_10_chIndex}" == "10" ]];then
	echo " which is 10"
else
	echo "bi_chcfg_10_chIndex is error"
fi


bi_chcfg_10_port=$(jq .${sku_name}.bi.chcfg[10].port  ${json_file_name})
echo -n "bi_chcfg_10_port is ${bi_chcfg_10_port}"
if [[ "${bi_chcfg_10_port}" == "\"${GPIO2}\"" || "${bi_chcfg_10_port}" == "\"${GPIO2_BASE}\"" ]];then
	echo " which is GPIO2"
else
	echo "bi_chcfg_10_port is error"
fi

bi_chcfg_10_pin=$(jq .${sku_name}.bi.chcfg[10].pin  ${json_file_name})
echo -n "bi_chcfg_10_pin is ${bi_chcfg_10_pin}"
if [[ "${bi_chcfg_10_pin}" == "14" || "${bi_chcfg_10_pin}" == "14U" ]];then
	echo " which is 14"
else
	echo "bi_chcfg_10_pin is error"
fi




#----------------------------------------------------------
echo ""

bi_chcfg_11_chIndex=$(jq .${sku_name}.bi.chcfg[11].chIndex  ${json_file_name})
echo -n "bi_chcfg_11_chIndex is ${bi_chcfg_11_chIndex}"
if [[ "${bi_chcfg_11_chIndex}" == "11" ]];then
	echo " which is 11"
else
	echo "bi_chcfg_11_chIndex is error"
fi


bi_chcfg_11_port=$(jq .${sku_name}.bi.chcfg[11].port  ${json_file_name})
echo -n "bi_chcfg_11_port is ${bi_chcfg_11_port}"
if [[ "${bi_chcfg_11_port}" == "\"${GPIO2}\"" || "${bi_chcfg_11_port}" == "\"${GPIO2_BASE}\"" ]];then
	echo " which is GPIO2"
else
	echo "bi_chcfg_11_port is error"
fi

bi_chcfg_11_pin=$(jq .${sku_name}.bi.chcfg[11].pin  ${json_file_name})
echo -n "bi_chcfg_11_pin is ${bi_chcfg_11_pin}"
if [[ "${bi_chcfg_11_pin}" == "12" || "${bi_chcfg_11_pin}" == "12U" ]];then
	echo " which is 12"
else
	echo "bi_chcfg_11_pin is error"
fi




#----------------------------------------------------------
echo ""

bi_chcfg_12_chIndex=$(jq .${sku_name}.bi.chcfg[12].chIndex  ${json_file_name})
echo -n "bi_chcfg_12_chIndex is ${bi_chcfg_12_chIndex}"
if [[ "${bi_chcfg_12_chIndex}" == "12" ]];then
	echo " which is 12"
else
	echo "bi_chcfg_12_chIndex is error"
fi


bi_chcfg_12_port=$(jq .${sku_name}.bi.chcfg[12].port  ${json_file_name})
echo -n "bi_chcfg_12_port is ${bi_chcfg_12_port}"
if [[ "${bi_chcfg_12_port}" == "\"${GPIO2}\"" || "${bi_chcfg_12_port}" == "\"${GPIO2_BASE}\"" ]];then
	echo " which is GPIO2"
else
	echo "bi_chcfg_12_port is error"
fi

bi_chcfg_12_pin=$(jq .${sku_name}.bi.chcfg[12].pin  ${json_file_name})
echo -n "bi_chcfg_12_pin is ${bi_chcfg_12_pin}"
if [[ "${bi_chcfg_12_pin}" == "0" || "${bi_chcfg_12_pin}" == "0U" ]];then
	echo " which is 0"
else
	echo "bi_chcfg_12_pin is error"
fi




#----------------------------------------------------------
echo ""

bi_chcfg_13_chIndex=$(jq .${sku_name}.bi.chcfg[13].chIndex  ${json_file_name})
echo -n "bi_chcfg_13_chIndex is ${bi_chcfg_13_chIndex}"
if [[ "${bi_chcfg_13_chIndex}" == "13" ]];then
	echo " which is 13"
else
	echo "bi_chcfg_13_chIndex is error"
fi


bi_chcfg_13_port=$(jq .${sku_name}.bi.chcfg[13].port  ${json_file_name})
echo -n "bi_chcfg_13_port is ${bi_chcfg_13_port}"
if [[ "${bi_chcfg_13_port}" == "\"${GPIO2}\"" || "${bi_chcfg_13_port}" == "\"${GPIO2_BASE}\"" ]];then
	echo " which is GPIO2"
else
	echo "bi_chcfg_13_port is error"
fi

bi_chcfg_13_pin=$(jq .${sku_name}.bi.chcfg[13].pin  ${json_file_name})
echo -n "bi_chcfg_13_pin is ${bi_chcfg_13_pin}"
if [[ "${bi_chcfg_13_pin}" == "13" || "${bi_chcfg_13_pin}" == "13U" ]];then
	echo " which is 13"
else
	echo "bi_chcfg_13_pin is error"
fi




#----------------------------------------------------------
echo ""

bi_chcfg_14_chIndex=$(jq .${sku_name}.bi.chcfg[14].chIndex  ${json_file_name})
echo -n "bi_chcfg_14_chIndex is ${bi_chcfg_14_chIndex}"
if [[ "${bi_chcfg_14_chIndex}" == "14" ]];then
	echo " which is 14"
else
	echo "bi_chcfg_14_chIndex is error"
fi


bi_chcfg_14_port=$(jq .${sku_name}.bi.chcfg[14].port  ${json_file_name})
echo -n "bi_chcfg_14_port is ${bi_chcfg_14_port}"
if [[ "${bi_chcfg_14_port}" == "\"${GPIO2}\"" || "${bi_chcfg_14_port}" == "\"${GPIO2_BASE}\"" ]];then
	echo " which is GPIO2"
else
	echo "bi_chcfg_14_port is error"
fi

bi_chcfg_14_pin=$(jq .${sku_name}.bi.chcfg[14].pin  ${json_file_name})
echo -n "bi_chcfg_14_pin is ${bi_chcfg_14_pin}"
if [[ "${bi_chcfg_14_pin}" == "18" || "${bi_chcfg_14_pin}" == "18U" ]];then
	echo " which is 18"
else
	echo "bi_chcfg_14_pin is error"
fi




#----------------------------------------------------------
echo ""

bi_chcfg_15_chIndex=$(jq .${sku_name}.bi.chcfg[15].chIndex  ${json_file_name})
echo -n "bi_chcfg_15_chIndex is ${bi_chcfg_15_chIndex}"
if [[ "${bi_chcfg_15_chIndex}" == "15" ]];then
	echo " which is 15"
else
	echo "bi_chcfg_15_chIndex is error"
fi


bi_chcfg_15_port=$(jq .${sku_name}.bi.chcfg[15].port  ${json_file_name})
echo -n "bi_chcfg_15_port is ${bi_chcfg_15_port}"
if [[ "${bi_chcfg_15_port}" == "\"${GPIO2}\"" || "${bi_chcfg_15_port}" == "\"${GPIO2_BASE}\"" ]];then
	echo " which is GPIO2"
else
	echo "bi_chcfg_15_port is error"
fi

bi_chcfg_15_pin=$(jq .${sku_name}.bi.chcfg[15].pin  ${json_file_name})
echo -n "bi_chcfg_15_pin is ${bi_chcfg_15_pin}"
if [[ "${bi_chcfg_15_pin}" == "27" || "${bi_chcfg_15_pin}" == "27U" ]];then
	echo " which is 27"
else
	echo "bi_chcfg_15_pin is error"
fi




