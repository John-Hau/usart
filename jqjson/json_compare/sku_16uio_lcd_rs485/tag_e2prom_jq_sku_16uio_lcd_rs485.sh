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
echo "tag e2prom"
echo "#####################"
echo ""

e2prom_0_Slaveaddr=$(jq .${sku_name}.e2prom[0].Slaveaddr ${json_file_name})
echo -n "e2prom_0_Slaveaddr is ${e2prom_0_Slaveaddr}"

if [[ "${e2prom_0_Slaveaddr}" == "\"0x50\"" ]];then
         echo " which is 0x50"
else
         echo "e2prom_0_Slaveaddr is error"
fi



#------------------------------------------------------------------------------------

e2prom_1_Slaveaddr=`jq .${sku_name}.e2prom[1].Slaveaddr ${json_file_name}`
echo -n "e2prom_1_Slaveaddr is ${e2prom_1_Slaveaddr}"

if [[ "${e2prom_1_Slaveaddr}" == "\"0x56\"" ]];then
         echo " which is 0x56"
else
         echo "e2prom_1_Slaveaddr is error"
fi







