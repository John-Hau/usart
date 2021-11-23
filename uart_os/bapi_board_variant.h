/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

#ifndef BAPI_BOARD_VARIANT_H_
#define BAPI_BOARD_VARIANT_H_
/**
 * \file
 * \brief
 * This file declares the Board ID related board API interface functions.
 * */
#include "baseplate.h"

#ifndef _BAPI_BOARD_VARIANT_DEFAULT_IMPLEMENTATION
#define _BAPI_BOARD_VARIANT_DEFAULT_IMPLEMENTATION 0
#endif

/**
 * \enum bapi_E_Board_Id_
 * \ingroup bapi_board_id
 * \brief
 * This enumeration abstracts the identification of the board variants of a board. Board Variants can
 * reflect differnetly populated peripherals (e.g. IO's). Identifying the board 
 * variant at runtime allows the same application software to run on all variants.
 * 
 * Type safety is an important criteria to avoid bugs. So the decision was taken to use an enumeration type
 * rather than a simple integer data type to enumerate a board variant.
 * The definition of this enumeration is board specific, because different boards have different board variants
 * and different designators. The implementation of this enumeration will have board specific designators. 
 */
#if defined (FS_IRMFCU)
  #include "boards/FS_IRMFCU/bapi_board_variant_FS_IRMFCU.h"

#elif defined (FS_IRMFCU_BL)
  #include "boards/FS_IRMFCU_BL/bapi_board_variant_FS_IRMFCU_BL.h"
#elif defined (FS_IRMVAV)
  #include "boards/FS_IRMVAV/bapi_board_variant_FS_IRMVAV.h"
#elif defined (FS_IMXRTEVAL)
  #include "boards/FS_IMXRTEVAL/bapi_board_variant_FS_IMXRTEVAL.h"
#elif defined (FS_BEATS_IO)
  #include "boards/FS_BEATS_IO/bapi_board_variant_FS_BEATS_IO.h"
#elif defined (FS_IMXRT_TSTAT)
  #include "boards/FS_IMXRT_TSTAT/bapi_board_variant_FS_IMXRT_TSTAT.h"
#elif defined (FS_IPVAV)
  #include "boards/FS_IPVAV/bapi_board_variant_FS_IPVAV.h"
#elif defined (FS_SNAP_ON_IO)
  #include "boards/FS_SNAP_ON_IO/bapi_board_variant_FS_SNAP_ON_IO.h"
#else
/* Default implementation for unknown boards. */
#define _BAPI_BOARD_VARIANT_DEFAULT_IMPLEMENTATION 1

enum bapi_E_Board_Variant_ {
  BOARD_VARIANT_UNKOWN = 0,
  BOARD_VARIANT_Count
};

C_INLINE enum bapi_E_Board_Variant_ bapi_bv_getBoardVariant() {
	return BOARD_VARIANT_UNKOWN;
}


#endif

#if !_BAPI_BOARD_VARIANT_DEFAULT_IMPLEMENTATION

C_FUNC enum bapi_E_Board_Variant_ bapi_bv_getBoardVariant();

C_FUNC void getBoardVariantName (uint8_t *modelName, uint8_t nameLen);

#endif

#endif /* BAPI_BOARD_VARIANT_H_ */
