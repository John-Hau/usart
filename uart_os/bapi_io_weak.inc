/*
 * bapi_io_weak.inc
 *
 *  Created on: 10.04.2013
 *      Author: e673505
 */

/** \file
 * \brief
 * Weak functions for applications that exclude the bapi-IO library (e.g. bootloader)
 */

#include "baseplate.h" 
 
#ifdef __GNUC__

  C_FUNC void _bo_configureGpioPins_() {return;}
  C_FUNC void _bo_configureGpioPins() __attribute__ ((weak, alias ("_bo_configureGpioPins_")));

  C_FUNC void _adc_configureGpioPins_() {return;}
  C_FUNC void _adc_configureGpioPins() __attribute__ ((weak, alias ("_adc_configureGpioPins_")));

  C_FUNC void _bi_configureGpioPins_() {return;}
  void _bi_configureGpioPins() __attribute__ ((weak, alias ("_bi_configureGpioPins_")));

  C_FUNC void _dac_configureGpioPins_() {return;}
  C_FUNC void _dac_configureGpioPins() __attribute__ ((weak, alias ("_dac_configureGpioPins_")));

  C_FUNC void _dac_initPwm_() {return;}
  C_FUNC void _dac_initPwm() __attribute__ ((weak, alias ("_dac_initPwm_")));

  C_FUNC void _readBoardVariant_() {return;}
  C_FUNC void _readBoardVariant() __attribute__ ((weak, alias ("_readBoardVariant_")));

#elif __IAR_SYSTEMS_ICC__

  C_FUNC __weak void _bo_configureGpioPins(void) {return;}
  C_FUNC __weak void _adc_configureGpioPins(void) {return;}
  C_FUNC __weak void _bi_configureGpioPins(void) {return;}
  C_FUNC __weak void _dac_configureGpioPins(void)  {return;}
  C_FUNC __weak void _dac_initPwm(void) {return;}
  C_FUNC __weak void _readBoardVariant(void) {return;}

#endif
