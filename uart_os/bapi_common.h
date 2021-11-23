/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

#ifndef bapi_common_H_
#define bapi_common_H_
/**
 * \file
 * \brief
 * This file declares the very basic board API interface functions. It is about the
 * hardware initialization and system tick functionality.
 * */

#include "baseplate.h"
#include "config.h"
/**
 * \ingroup bapi_common
 * \brief Initialize the hardware board.
 *
 * Initializes the Micro Controller and configures the gpio multiplexing according to the the board design.
 * It also starts the system tick timer and enables the system tick interrupt that calls the
 * system tick interrupt service routine frequently (SysTick_Handler() on ARM processors).
 * The system tick frequency must be defined in the product configuration file by the symbol
 * BAPI_RQ_SYSTEM_TICK_FREQUENCY_HZ.
 *
 * bapi_initHardware() is the first function to be called by the main(void);
 *
 * __Code Example:__
 ~~~~~~~~{.c}
 #include "boards/board-api/bapi_common.h"

 int main (void) {
   // Initialize hardware, before anything else.
   bapi_initHardware();
         :
 }
 ~~~~~~~~
 */
C_FUNC void bapi_initHardware();

C_FUNC void bapi_initClocks(void);

C_FUNC void  bapi_ConfigMPU(void);
#if defined (FS_BEATS_IO)
C_FUNC void  BOARD_ConfigMPU(void);
#endif
/**
 * \ingroup bapi_common
 * \brief
 * This function is supposed to be be called upon fatal error (like stack overflow). The
 * behavior is board dependent and might flash an LED, or log the
 * file name and line number in flash memory. */
C_FUNC NORETURN void bapi_fatalError(char const* file, const unsigned int line);
#if defined (FS_BEATS_IO)|| defined (FS_SNAP_ON_IO)
C_FUNC NORETURN void bapi_configassert_Error(char const* file, const unsigned int line);
#endif
/**
 * \ingroup bapi_systick
 * \brief
 * The data type to be used for the system tick counter. It is an unsigned value.
 * */
typedef uint32_t bapi_SystemTick_t;

/**
 * \ingroup bapi_systick
 * \brief
 * The maximum value of the system tick counter before overflow.
 */
#define BAPI_TICK_TYPE_MAX UINT32_MAX


#if (TARGET_RTOS != RTOS_NoRTOS)
  /**
   * \ingroup bapi_systick
   * \brief
   * The system tick function that needs to be called by the RTOS upon a system tick interrupt,
   * so that the board api can perform it's local system tick tasks like incrementing the local
   * system tick counter and calling the bapi_systemTickIsrHook(bapi_SystemTick_t systemTick)
   * function.
   */
  C_FUNC void bapi_SysTick_Handler();
#endif

/**
 * \ingroup bapi_systick
 * \brief
 * Retrieve the Board API local system tick counter
 *
 * */
C_FUNC bapi_SystemTick_t bapi_getSystemTick();


/**
 * \ingroup bapi_systick
 * \brief
 * Callback function type that can be hooked into the system tick ISR.
 */
typedef void (*bapi_systemTickCallback_t)(bapi_SystemTick_t systemTick);

/**
 * \ingroup bapi_systick
 * \brief
 * Establish a callback function that will be called upon each system tick.
 *
 * @param[in] systemTickCallback The callback function to be established.
 * @return The callback function that was established before. May be null.
 *
 * \note In case that the returned old callback is not null, the new callback
 *  function should also call this old one.
 *
 */
C_FUNC bapi_systemTickCallback_t bapi_systemTickSetCallback(bapi_systemTickCallback_t systemTickCallback);


/**
 * \ingroup bapi_systick
 * \brief
 * Converts the current System Tick counter into milliseconds. Note that overflows
 * may happen.
 *
 * \return The system tick converted into milliseconds.
 */
C_INLINE MsecType bapi_systemTick2Msec(
  bapi_SystemTick_t systemTickCount /**< The counter value to be converted into milliseconds. */
  ) {
  return (systemTickCount * (MSEC_PER_SEC / BAPI_RQ_SYSTEM_TICK_FREQUENCY_HZ));
}

/**
 * \ingroup bapi_common
 * \brief Safely prints the contents of memory block as hex string into memory.
 *
 * This function checks if the memory block is valid for the MCU. If not,
 *  an error message will be printed instead.
 *
 * @param memblock [in] pointer to the memory block to be printed
 * @param size     [in] the size of the memory block to be printed
 * @param buffer   [in] the buffer where to print
 * @param buffer   [in] the size of the buffer
 *
 * @return number of printed characters.
 */
C_FUNC unsigned bapi_snprintmem(const uint8_t *memblock, unsigned size, char *buffer, unsigned buffersize);


/**
 * \ingroup bapi_common
 * \brief Assert that a memory block is in valid RAM or FLASH range. of the MCU.
 *
 * @param memblock [in] pointer to the memory block to be asserted
 * @param size     [in] the size of the memory block to be asserted
 * @return true, if the memory block is in valid RAM or FLASH range of the MCU,
 *   or if it is a zero length block. Otherwise false.
 */
C_FUNC bool bapi_isValidMemblock(const uint8_t *memblock, unsigned size);


#endif // #ifndef bapi_common_H_
