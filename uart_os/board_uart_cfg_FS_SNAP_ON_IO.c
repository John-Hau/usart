/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

/**
 * \file
 * \brief Provides the FS_IRMFCU board specific configuration data and runtime
 * variables according to the declarations in board_usart_cfg.h.
 *
 * This data is used by the vendor specific
 * USART Board API implementation file bapi_uart_MCU_VENDOR_NXP.c.
 */

#include <stddef.h>
#include <stdint.h>
#include "baseplate.h"


#include "boards/board-api/bapi_uart.h"
#include "boards/vendors/MCU_VENDOR_NXP/board_uart_cfg_MCU_VENDOR_NXP.h"
#include "initialize_FS_SNAP_ON_IO.h"


#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

  #include "fsl_device_registers.h"
  #include "fsl_iomuxc.h"
  #include "fsl_gpio.h"

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif


const struct _bapi_uart_properties _uart_properties[bapi_E_UartCount] = {
    {LPUART1_BASE, LPUART1_IRQn, LPUART1_IRQn, fslUartTypeIndexLpuart}   /* [0] = bapi_E_UART0  */
  , {LPUART2_BASE, LPUART2_IRQn, LPUART2_IRQn, fslUartTypeIndexLpuart}   /* [1] = bapi_E_UART1  */
  , {LPUART3_BASE, LPUART3_IRQn, LPUART3_IRQn, fslUartTypeIndexLpuart}   /* [2] = bapi_E_UART2  */
  , {LPUART4_BASE, LPUART4_IRQn, LPUART4_IRQn, fslUartTypeIndexLpuart}   /* [3] = bapi_E_UART3  */
  , {LPUART5_BASE, LPUART5_IRQn, LPUART5_IRQn, fslUartTypeIndexLpuart}   /* [4] = bapi_E_UART4  */
  , {LPUART6_BASE, LPUART6_IRQn, LPUART6_IRQn, fslUartTypeIndexLpuart}   /* [5] = bapi_E_UART5  */
  , {LPUART7_BASE, LPUART7_IRQn, LPUART7_IRQn, fslUartTypeIndexLpuart}   /* [5] = bapi_E_UART5  */
  , {LPUART8_BASE, LPUART8_IRQn, LPUART8_IRQn, fslUartTypeIndexLpuart}   /* [5] = bapi_E_UART5  */
};



#if LPUART_INSTANCE_COUNT
const struct _fsl_uart_instance_properties _fsl_lpuartInstanceProperties[LPUART_INSTANCE_COUNT] = {
		  {bapi_E_Uart1, 0}  /* [1] -> instance UART1_IDX */
		  ,{bapi_E_Uart2, 0}  /* [2] -> instance UART2_IDX */
		  ,{bapi_E_Uart3, 0}  /* [3] -> instance UART3_IDX */
		  ,{bapi_E_Uart4, 0}  /* [4] -> instance UART4_IDX */
		  ,{bapi_E_Uart5, 0}  /* [5] -> instance UART5_IDX */
		  ,{bapi_E_Uart6, 0}  /* [5] -> instance UART5_IDX */
		  ,{bapi_E_Uart7, 0}  /* [5] -> instance UART5_IDX */
		  ,{bapi_E_Uart8, 0}  /* [5] -> instance UART5_IDX */
};
#endif

struct _uart_Callbacks _uart_callbacks[bapi_E_UartCount] = {
   {0, {0,0}}
  ,{0, {0,0}}
  ,{0, {0,0}}
  ,{0, {0,0}}
  ,{0, {0,0}}
  ,{0, {0,0}}
  ,{0, {0,0}}
  ,{0, {0,0}}
};

uint32_t _uart_Baudrate[bapi_E_UartCount] = {0, 0, 0, 0, 0, 0, 0, 0};

uint8_t _uart_irq_disable_count[bapi_uart_IRQT_Count][bapi_E_UartCount] =
  {
    /* UART0, UART1, UART2, UART3, UART4, UART5 */
    {    0,     0,     0,     0,     0,     0 , 0, 0   } /* bapi_uart_IRQT_RX */
   ,{    1,     1,     1,     1,     1,     1 , 1, 1   } /* bapi_uart_IRQT_TX */
  };

uint8_t _uart_disable_tx_count[bapi_E_UartCount] =
    /* UART0, UART1, UART2, UART3, UART4, UART5 */
    {    1,     1,     1,     1,     1,     1 , 1, 1   };/* bapi_uart disable transmitter counter */

uint8_t _uart_disable_rx_count[bapi_E_UartCount] =
	/* UART0, UART1, UART2, UART3, UART4, UART5 */
	{    1,     1,     1,     1,     1,     1, 1, 1    };/* bapi_uart disable receiver counter */

bapi_E_UartMode _bapi_uart_mode[bapi_E_UartCount] = {
  arm_USART_MODE_UNINITIALIZED,
  arm_USART_MODE_UNINITIALIZED,
  arm_USART_MODE_UNINITIALIZED,
  arm_USART_MODE_UNINITIALIZED,
  arm_USART_MODE_UNINITIALIZED,
  arm_USART_MODE_UNINITIALIZED,
  arm_USART_MODE_UNINITIALIZED,
  arm_USART_MODE_UNINITIALIZED
};

/* This concept supports currently only one flag per UART. To be enhanced if required. */
const struct _bapi_uart_flagProperties _uartFlagProperties[bapi_E_UartCount] = {
  {0, bapi_E_InvalidInterfaceFlag, 0},
  {0, bapi_E_InvalidInterfaceFlag, 0},
  {GPIO1, bapi_E_RS485_EnableTransmitter, 21u},
  {0, bapi_E_InvalidInterfaceFlag, 0},
  {0, bapi_E_InvalidInterfaceFlag, 0},
  {0, bapi_E_InvalidInterfaceFlag, 0},
  {GPIO3, bapi_E_RS485_EnableTransmitter, 7u},       /* UART7_BACnet_TxEnable */
  {0, bapi_E_InvalidInterfaceFlag, 0}
};

C_INLINE void configureGpioPinAsOutput(const struct _bapi_uart_flagProperties* flagProperties) {
  //TODO: COnfigure as output if in case any extra pin control is needed by uarts ; need to define in _bapi_uart_flagProperties as well

}


void _uart_configureGpioPins()
{

  /************************ UART0 ********************************************/


  /************************ LPUART1 used for serial debug console ********************************************/

#if 0
	  IOMUXC_SetPinMux(
	      IOMUXC_GPIO_AD_B0_12_LPUART1_TX,        /* GPIO_AD_B0_12 is configured as LPUART1_TX */
	      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	  IOMUXC_SetPinMux(
	      IOMUXC_GPIO_AD_B0_13_LPUART1_RX,        /* GPIO_AD_B0_13 is configured as LPUART1_RX */
	      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	  IOMUXC_SetPinConfig(
	      IOMUXC_GPIO_AD_B0_12_LPUART1_TX,        /* GPIO_AD_B0_12 PAD functional properties : */
	      0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
	                                                 Drive Strength Field: R0/6
	                                                 Speed Field: medium(100MHz)
	                                                 Open Drain Enable Field: Open Drain Disabled
	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	                                                 Pull / Keep Select Field: Keeper
	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
	                                                 Hyst. Enable Field: Hysteresis Disabled */
	  IOMUXC_SetPinConfig(
	      IOMUXC_GPIO_AD_B0_13_LPUART1_RX,        /* GPIO_AD_B0_13 PAD functional properties : */
	      0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
	                                                 Drive Strength Field: R0/6
	                                                 Speed Field: medium(100MHz)
	                                                 Open Drain Enable Field: Open Drain Disabled
	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	                                                 Pull / Keep Select Field: Keeper
	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
	                                                 Hyst. Enable Field: Hysteresis Disabled */
#else
	  IOMUXC_SetPinMux(
	      IOMUXC_GPIO_B1_00_LPUART4_TX,           /* GPIO_B1_00 is configured as LPUART4_TX */
	      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	  IOMUXC_SetPinMux(
	      IOMUXC_GPIO_B1_01_LPUART4_RX,           /* GPIO_B1_01 is configured as LPUART4_RX */
	      0U);                                    /* Software Input On Field: Input Path is determined by functionality */

	  IOMUXC_SetPinConfig(
		  IOMUXC_GPIO_B1_00_LPUART4_TX,           /* GPIO_B1_00 PAD functional properties : */
	      0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
	                                                 Drive Strength Field: R0/6
	                                                 Speed Field: medium(100MHz)
	                                                 Open Drain Enable Field: Open Drain Disabled
	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	                                                 Pull / Keep Select Field: Keeper
	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
	                                                 Hyst. Enable Field: Hysteresis Disabled */
	  IOMUXC_SetPinConfig(
	     IOMUXC_GPIO_B1_01_LPUART4_RX,            /* GPIO_B1_01 PAD functional properties : */
	      0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
	                                                 Drive Strength Field: R0/6
	                                                 Speed Field: medium(100MHz)
	                                                 Open Drain Enable Field: Open Drain Disabled
	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	                                                 Pull / Keep Select Field: Keeper
	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
	                                                 Hyst. Enable Field: Hysteresis Disabled */
#endif
  /************************ UART2 ********************************************/


  /************************ UART3 ********************************************/
	  /************************ LPUART7 used for mstp uart ********************************************/
#if 1//HAS_MSTP






//=======================================================================================================================================
#if RS485_ENABLE


	      IOMUXC_SetPinMux(
	 			IOMUXC_GPIO_SD_B1_08_LPUART7_TX,        /* GPIO_SD_B1_08 is configured as LPUART1_TX */
	 	      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	 	  IOMUXC_SetPinMux(
	 			 IOMUXC_GPIO_SD_B1_09_LPUART7_RX,        /* GPIO_SD_B1_09 is configured as LPUART1_RX */
	 	      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	 	  IOMUXC_SetPinConfig(
	 			 IOMUXC_GPIO_SD_B1_08_LPUART7_TX,        /* GPIO_SD_B1_08 PAD functional properties : */
	 	      0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
	 	                                                 Drive Strength Field: R0/6
	 	                                                 Speed Field: medium(100MHz)
	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	                                                 Pull / Keep Select Field: Keeper
	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
	 	  IOMUXC_SetPinConfig(
	 			 IOMUXC_GPIO_SD_B1_09_LPUART7_RX,        /* GPIO_SD_B1_09 PAD functional properties : */
	 	      0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
	 	                                                 Drive Strength Field: R0/6
	 	                                                 Speed Field: medium(100MHz)
	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	                                                 Pull / Keep Select Field: Keeper
	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
	 	 /* RTS_b -> Manually controlled as binary output to switch RS485 Transmitter */
	 	 //configureGpioPinAsOutput(&_uartFlagProperties[3]);

	 	 	IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_07_GPIO3_IO07 , 0U);
	 	 	IOMUXC_SetPinConfig(
	 	 			IOMUXC_GPIO_SD_B1_07_GPIO3_IO07,
	 	 	    0x10B0u);
	 	   gpio_pin_config_t portpin_config = {
            kGPIO_DigitalOutput,
            0,
            kGPIO_NoIntmode, //Default
          };
	 	  GPIO_PinInit(GPIO3, 7, &portpin_config);



	 	 //GPIO_PinWrite(GPIO3, 7, 0U);
		//LPUART_EnableInterrupts(LPUART7, kLPUART_RxDataRegFullInterruptEnable);


	 	 bapi_irq_setPrio(LPUART7_IRQn,bapi_E_IrqLowestPrio+10);

#endif

//---------------------------------------------------------------------------------------------------------------------------------------------------

#if 0
		 	IOMUXC_SetPinMux(
		 			IOMUXC_GPIO_B0_08_LPUART3_TX,        /* GPIO_SD_B1_08 is configured as LPUART1_TX */
		 	      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
		 	  IOMUXC_SetPinMux(
		 			 IOMUXC_GPIO_B0_09_LPUART3_RX,        /* GPIO_SD_B1_09 is configured as LPUART1_RX */
		 	      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
		 	  IOMUXC_SetPinConfig(
		 			 IOMUXC_GPIO_B0_08_LPUART3_TX,        /* GPIO_SD_B1_08 PAD functional properties : */
		 	      0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
		 	                                                 Drive Strength Field: R0/6
		 	                                                 Speed Field: medium(100MHz)
		 	                                                 Open Drain Enable Field: Open Drain Disabled
		 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
		 	                                                 Pull / Keep Select Field: Keeper
		 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
		 	                                                 Hyst. Enable Field: Hysteresis Disabled */
		 	  IOMUXC_SetPinConfig(
		 			 IOMUXC_GPIO_B0_09_LPUART3_RX,        /* GPIO_SD_B1_09 PAD functional properties : */
		 	      0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
		 	                                                 Drive Strength Field: R0/6
		 	                                                 Speed Field: medium(100MHz)
		 	                                                 Open Drain Enable Field: Open Drain Disabled
		 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
		 	                                                 Pull / Keep Select Field: Keeper
		 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
		 	                                                 Hyst. Enable Field: Hysteresis Disabled */
		 	 /* RTS_b -> Manually controlled as binary output to switch RS485 Transmitter */
		 	 //configureGpioPinAsOutput(&_uartFlagProperties[3]);

		 	 	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_05_GPIO1_IO21 , 0U);
		 	 	IOMUXC_SetPinConfig(
		 	 			IOMUXC_GPIO_AD_B1_05_GPIO1_IO21,
		 	 	    0x10B0u);
		 	   gpio_pin_config_t portpin_config = {
	          kGPIO_DigitalOutput,
	          0,
	          kGPIO_NoIntmode, //Default
	        };
		 	  GPIO_PinInit(GPIO1, 21U, &portpin_config);
		 	  bapi_irq_setPrio(LPUART3_IRQn,bapi_E_IrqLowestPrio+10);
#endif

//===============================================================================================================================================

#else //#if HAS_MSTP
	 	 IOMUXC_SetPinMux(
	 	       IOMUXC_GPIO_B1_04_ENET_RX_DATA00,       /* GPIO_B1_04 is configured as ENET_RX_DATA00 */
	 	       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	 	   IOMUXC_SetPinMux(
	 	       IOMUXC_GPIO_B1_05_ENET_RX_DATA01,       /* GPIO_B1_05 is configured as ENET_RX_DATA01 */
	 	       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	 	   IOMUXC_SetPinMux(
	 	       IOMUXC_GPIO_B1_06_ENET_RX_EN,           /* GPIO_B1_06 is configured as ENET_RX_EN */
	 	       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	 	   IOMUXC_SetPinMux(
	 	       IOMUXC_GPIO_B1_07_ENET_TX_DATA00,       /* GPIO_B1_07 is configured as ENET_TX_DATA00 */
	 	       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	 	   IOMUXC_SetPinMux(
	 	       IOMUXC_GPIO_B1_08_ENET_TX_DATA01,       /* GPIO_B1_08 is configured as ENET_TX_DATA01 */
	 	       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	 	   IOMUXC_SetPinMux(
	 	       IOMUXC_GPIO_B1_09_ENET_TX_EN,           /* GPIO_B1_09 is configured as ENET_TX_EN */
	 	       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	 	   IOMUXC_SetPinMux(
	 	       IOMUXC_GPIO_B1_10_ENET_REF_CLK,         /* GPIO_B1_10 is configured as ENET_REF_CLK */
	 	       1U);                                    /* Software Input On Field: Force input path of pad GPIO_B1_10 */
#if 0
	 	  IOMUXC_SetPinMux(
	 	      IOMUXC_GPIO_B1_11_ENET_RX_ER,           /* GPIO_B1_11 is configured as ENET_RX_ER */
	 	      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
#endif //#if 0
	 	  // IOMUXC_SetPinMux(
	 	  //     IOMUXC_GPIO_B1_14_ENET_MDC,             /* GPIO_B1_14 is configured as ENET_MDC */
	 	  //     0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	 	  // IOMUXC_SetPinMux(
	 	   //    IOMUXC_GPIO_B1_15_ENET_MDIO,            /* GPIO_B1_15 is configured as ENET_MDIO */
	 	   //    0U);


	 	  IOMUXC_SetPinConfig(
	 	      IOMUXC_GPIO_B1_04_ENET_RX_DATA00,       /* GPIO_B1_04 PAD functional properties : */
	 	      0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
	 	                                                 Drive Strength Field: R0/5
	 	                                                 Speed Field: max(200MHz)
	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	                                                 Pull / Keep Select Field: Pull
	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Up
	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
	 	  IOMUXC_SetPinConfig(
	 	      IOMUXC_GPIO_B1_05_ENET_RX_DATA01,       /* GPIO_B1_05 PAD functional properties : */
	 	      0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
	 	                                                 Drive Strength Field: R0/5
	 	                                                 Speed Field: max(200MHz)
	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	                                                 Pull / Keep Select Field: Pull
	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Up
	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
	 	  IOMUXC_SetPinConfig(
	 	      IOMUXC_GPIO_B1_06_ENET_RX_EN,           /* GPIO_B1_06 PAD functional properties : */
	 	      0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
	 	                                                 Drive Strength Field: R0/5
	 	                                                 Speed Field: max(200MHz)
	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	                                                 Pull / Keep Select Field: Pull
	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Up
	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
	 	  IOMUXC_SetPinConfig(
	 	      IOMUXC_GPIO_B1_07_ENET_TX_DATA00,       /* GPIO_B1_07 PAD functional properties : */
	 	      0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
	 	                                                 Drive Strength Field: R0/5
	 	                                                 Speed Field: max(200MHz)
	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	                                                 Pull / Keep Select Field: Pull
	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Up
	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
	 	  IOMUXC_SetPinConfig(
	 	      IOMUXC_GPIO_B1_08_ENET_TX_DATA01,       /* GPIO_B1_08 PAD functional properties : */
	 	      0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
	 	                                                 Drive Strength Field: R0/5
	 	                                                 Speed Field: max(200MHz)
	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	                                                 Pull / Keep Select Field: Pull
	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Up
	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
	 	  IOMUXC_SetPinConfig(
	 	      IOMUXC_GPIO_B1_09_ENET_TX_EN,           /* GPIO_B1_09 PAD functional properties : */
	 	      0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
	 	                                                 Drive Strength Field: R0/5
	 	                                                 Speed Field: max(200MHz)
	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	                                                 Pull / Keep Select Field: Pull
	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Up
	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
	 	  IOMUXC_SetPinConfig(
	 	      IOMUXC_GPIO_B1_10_ENET_REF_CLK,         /* GPIO_B1_10 PAD functional properties : */
	 	      0x31u);                                 /* Slew Rate Field: Fast Slew Rate
	 	                                                 Drive Strength Field: R0/6
	 	                                                 Speed Field: low(50MHz)
	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	                                                 Pull / Keep Enable Field: Pull/Keeper Disabled
	 	                                                 Pull / Keep Select Field: Keeper
	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
#if 0
	 	  IOMUXC_SetPinConfig(
	 	      IOMUXC_GPIO_B1_11_ENET_RX_ER,           /* GPIO_B1_11 PAD functional properties : */
	 	      0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
	 	                                                 Drive Strength Field: R0/5
	 	                                                 Speed Field: max(200MHz)
	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	                                                 Pull / Keep Select Field: Pull
	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Up
	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
#endif //#if 0
	 	  IOMUXC_SetPinConfig(
	 	      IOMUXC_GPIO_EMC_40_ENET_MDC,            /* GPIO_EMC_40 PAD functional properties : */
	 	      0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
	 	                                                 Drive Strength Field: R0/5
	 	                                                 Speed Field: max(200MHz)
	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	                                                 Pull / Keep Select Field: Pull
	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Up
	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
	 	  IOMUXC_SetPinConfig(
	 	      IOMUXC_GPIO_EMC_41_ENET_MDIO,           /* GPIO_EMC_41 PAD functional properties : */
	 	      0xB829u);                               /* Slew Rate Field: Fast Slew Rate
	 	                                                 Drive Strength Field: R0/5
	 	                                                 Speed Field: low(50MHz)
	 	                                                 Open Drain Enable Field: Open Drain Enabled
	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	                                                 Pull / Keep Select Field: Pull
	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Up
	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
#endif //#if HAS_MSTP
  /************************ UART4 ********************************************/


  /************************ UART5 ********************************************/

  /************************ UART6 ********************************************/

  /************************ UART7 ********************************************/
 //GPIO3 IO18 as SYLK RX
 //GPIO4 IO31 as SYLK TX
#if 0
	 	  IOMUXC_SetPinMux(
	 			 IOMUXC_GPIO_EMC_31_LPUART7_TX,        /* GPIO4 IO31 is configured as LPUART1_TX */
	 	  	 	      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	 	  	 	  IOMUXC_SetPinMux(
	 	  	 			IOMUXC_GPIO_EMC_32_LPUART7_RX,        /* GPIO3 IO18 is configured as LPUART1_RX */
	 	  	 	      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	 	  	 	  IOMUXC_SetPinConfig(
	 	  	 			IOMUXC_GPIO_EMC_31_LPUART7_TX,        /* GPIO_AD_B0_12 PAD functional properties : */
	 	  	 	      0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
	 	  	 	                                                 Drive Strength Field: R0/6
	 	  	 	                                                 Speed Field: medium(100MHz)
	 	  	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	  	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	  	 	                                                 Pull / Keep Select Field: Keeper
	 	  	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
	 	  	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
	 	  	 	  IOMUXC_SetPinConfig(
	 	  	 			IOMUXC_GPIO_EMC_32_LPUART7_RX,        /* GPIO_AD_B0_13 PAD functional properties : */
	 	  	 	      0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
	 	  	 	                                                 Drive Strength Field: R0/6
	 	  	 	                                                 Speed Field: medium(100MHz)
	 	  	 	                                                 Open Drain Enable Field: Open Drain Disabled
	 	  	 	                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
	 	  	 	                                                 Pull / Keep Select Field: Keeper
	 	  	 	                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
	 	  	 	                                                 Hyst. Enable Field: Hysteresis Disabled */
#endif
  /************************ UART8 ********************************************/

}

/* Driver Capabilities */
const struct _ARM_USART_CAPABILITIES _bapi_uartCapabilities[bapi_E_UartCount] = {
/*UART[0]*/  {
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
#if FSL_FEATURE_UART_HAS_MODEM_SUPPORT
    1, /* RTS Flow Control available */
    1, /* CTS Flow Control available */
#else
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
#endif
    1, /* Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE */
    0, /* Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USART_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USART_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USART_EVENT_DCD */
    0  /* Signal RI change event: \ref ARM_USART_EVENT_RI */
/*UART[1]*/  },{
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
#if 0  /* This board does not support RTS, CTS for UART[1] */
    1, /* RTS Flow Control available */
    1, /* CTS Flow Control available */
#else
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
#endif
    1, /* Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE */
    0, /* Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USART_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USART_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USART_EVENT_DCD */
    0  /* Signal RI change event: \ref ARM_USART_EVENT_RI */
/*UART[2]*/  },{
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
#if FSL_FEATURE_UART_HAS_MODEM_SUPPORT
    1, /* RTS Flow Control available */
    1, /* CTS Flow Control available */
#else
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
#endif
    1, /* Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE */
    0, /* Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USART_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USART_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USART_EVENT_DCD */
    0  /* Signal RI change event: \ref ARM_USART_EVENT_RI */
/*UART[3]*/  },{
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
#if 0  /* This board does not support RTS, CTS for UART[3] */
    1, /* RTS Flow Control available */
    1, /* CTS Flow Control available */
#else
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
#endif
    1, /* Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE */
    0, /* Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USART_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USART_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USART_EVENT_DCD */
    0  /* Signal RI change event: \ref ARM_USART_EVENT_RI */
/*UART[4]*/  },{
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
#if FSL_FEATURE_UART_HAS_MODEM_SUPPORT
    1, /* RTS Flow Control available */
    1, /* CTS Flow Control available */
#else
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
#endif
    1, /* Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE */
    0, /* Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USART_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USART_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USART_EVENT_DCD */
    0  /* Signal RI change event: \ref ARM_USART_EVENT_RI */
/*UART[5]*/  },{
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
#if 0  /* This board does not support RTS, CTS for UART[5] */
    1, /* RTS Flow Control available */
    1, /* CTS Flow Control available */
#else
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
#endif
    1, /* Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE */
    0, /* Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USART_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USART_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USART_EVENT_DCD */
    0  /* Signal RI change event: \ref ARM_USART_EVENT_RI */
  }
};
