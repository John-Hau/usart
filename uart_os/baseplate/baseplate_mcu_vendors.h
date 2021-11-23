/*
 * baseplate.h
 *
 *  Created on: 03.04.2013
 *      Author: e673505
 */

#ifndef BASEPLATE_MCU_VENDORS_H_
#define BASEPLATE_MCU_VENDORS_H_

/* MCU silicon vendors options. Will be used by CMake auto generated hardware-board.h */
#define MCU_VENDOR_NONE         0
#define MCU_VENDOR_SILABS       1
#define MCU_VENDOR_FREESCALE    2
#define MCU_VENDOR_STM          3
#define MCU_VENDOR_ATMEL        4
#define MCU_VENDOR_NXP          5

/* Valid MCU_CORE_TYPES. MCU Core impacts FreeRTOS configuration. See board_FreeRTOSConfig.h */
#define MCU_CORE_TYPE_ARM_CM0   1
#define MCU_CORE_TYPE_ARM_CM3   2
#define MCU_CORE_TYPE_ARM_CM4   3
#define MCU_CORE_TYPE_ARM_CM4F  4
#define MCU_CORE_TYPE_ARM_CM7   5

#endif /* BASEPLATE_MCU_VENDORS_H_ */
