/*
 * osCom.h
 *
 *  Created on: 07.04.2013
 *      Author: Wolfgang
 */

#ifndef osIoBi_H_
#define osIoBi_H_


#include "baseplate.h"
//#include "boards/board-api/bapi_uart.h"
//#include "cmsis-driver/Driver_USART.h"

#ifdef __IAR_SYSTEMS_ICC__
  #include <LowLevelIOInterface.h>
#else
  #include <unistd.h>
#endif

#include "boards/board-api/bapi_io.h"
#include "rtos/cmsis-rtos/cmsis_os_redirect.h"

/**
 * \file
 * This file declares the ARM CMSIS RTOS Binary Input Extension API.
 */


struct osBiGroup_;
typedef struct osBiGroup_* osBiGroupId;

typedef uint32_t bitfield_base_t;

enum {
#if defined (FS_BEATS_IO)
   BI_MAIL_Q_SIZE = 1
#else
   BI_MAIL_Q_SIZE = 2
#endif
  ,BITS_PER_BYTE = 8
  ,BITFIELD_ARRAY_SIZE
    = (bapi_bi_E_Ch_Count + (BITS_PER_BYTE * sizeof(bitfield_base_t)) - 1) / (BITS_PER_BYTE * sizeof(bitfield_base_t))
};

typedef struct _osBiGroupMembers_ {
#ifdef BIGROUP_DEBUG
  bigroup_E_MailTag m_mailTag;
#endif
  bitfield_base_t m_biStateChangeBitfield[BITFIELD_ARRAY_SIZE];   /**< Bit field for the BI channel that changed the state. */
  bitfield_base_t m_biStateBitfield[BITFIELD_ARRAY_SIZE];         /**< Bit field for the state of a BI channel. */
  bitfield_base_t m_biPresenceBitfield[BITFIELD_ARRAY_SIZE];      /**< Bit field for the presence of a BI channels. When the
                                                                   * corresponding bit is 1, the BI is in the group. */
  uint64_t pulse_counter[bapi_bi_E_Ch_Count]; //h242608

#ifdef __cplusplus
  inline void addBiChannel(bapi_E_BiChannel biChannel) {
    size_t index = biChannel / (BITS_PER_BYTE * sizeof(bitfield_base_t));
    size_t bitNo = biChannel % (BITS_PER_BYTE * sizeof(bitfield_base_t));
    m_biPresenceBitfield[index] |= (1 << bitNo);
  }

  inline void removeBiChannel(bapi_E_BiChannel biChannel) {
    size_t index = biChannel / (BITS_PER_BYTE * sizeof(bitfield_base_t));
    size_t bitNo = biChannel % (BITS_PER_BYTE * sizeof(bitfield_base_t));
    m_biPresenceBitfield[index] &= ~(1 << bitNo);
  }

  inline void setBiChannelState(bapi_E_BiChannel biChannel) {
    ASSERT(bapi_irq_isInterruptContext()); /* This must only be called from the ISR */
    size_t index = biChannel / (BITS_PER_BYTE * sizeof(bitfield_base_t));
    size_t bitNo = biChannel % (BITS_PER_BYTE * sizeof(bitfield_base_t));
    m_biStateBitfield[index] |= (1 << bitNo);
    m_biStateChangeBitfield[index] |= (1 << bitNo);
  }

  inline void resetBiChannelState(bapi_E_BiChannel biChannel) {
    ASSERT(bapi_irq_isInterruptContext()); /* This must only be called from the ISR */
    size_t index = biChannel / (BITS_PER_BYTE * sizeof(bitfield_base_t));
    size_t bitNo = biChannel % (BITS_PER_BYTE * sizeof(bitfield_base_t));
    m_biStateBitfield[index] &= ~(1 << bitNo);
    m_biStateChangeBitfield[index] |= (1 << bitNo);
  }

//h242608
  inline void resetBiPulseCounter(bapi_E_BiChannel biChannel) {
    pulse_counter[biChannel] = 0;
  }

  inline void setBiPulseCounter(bapi_E_BiChannel biChannel, uint64_t count) {
    pulse_counter[biChannel] = count;
  }


#endif /* #ifdef __cplusplus */

} _osBiGroupMembers;
typedef struct _osBiGroupMembers_ osBiStateChangeEvents;

/**
 * \ingroup cmsis_os_ext_bi
 * \brief Creates a group of Binary Inputs. State Change Events will be
 * generated for all active Binary Input Groups.
 *
 * \note Works also in a Non RTOS environment.
 *
 * With this grouping you can have individual threads handling State Change
 * Events for a particular set of Binary Inputs. If all Binary Inputs state changes
 * should be handled by a single thread, you can define one Binary Input group
 * and place all Binary Inputs into this single group.
 *
 * A thread can wait for State Change Events of a particular group by calling
 * osMailGet(osMailQId queue_id, uint32_t), where queue_id has to be obtained
 * from by calling osBiGroupGetMailQ(osBiGroupId biGroup).
 * \sa osBiGroupActivate(osBiGroupId biGroup).
 *
 * A particular Binary Input can only be associated with one
 * group. If a Binary Input is put in multiple groups, it will
 * only send State Change Events to the mail queue of the group that was
 * activated first.
 */
C_FUNC osBiGroupId osBiGroupCreate(
    const bapi_E_BiChannel* groupMembers /**< [in] pointer to an array of Binary Inputs that will belong to the new group */
  , unsigned int groupMemberCount        /**< [in] size of the groupMembers array. */
  );

/**
 * \ingroup cmsis_os_ext_bi
 * \brief This activates the State Change Event generation for all
 * Binary Inputs that belong to a group.
 *
 * \note Works also in a Non RTOS environment.
 *
 * osMailGet(osMailQId queue_id, uint32_t) will not receive
 * any events before this function was called after creation or de-activation.
 * \note Change State Events will be sent to the mail queue that is associstad
 * with the Binary Input group. This mail queue can be obtained by calling
 * osBiGroupGetMailQ(osBiGroupId biGroup).
 *
 * \return osOK, if the activation of State Change Event generation for all Binary
 * Inputs was successfully established. osErrorOS if there is at
 * least one Binary Input for which the State Change Event could not be
 * established, because the Binary Input belongs to another group as well.
 * In that case the other group keeps State Change Event generation for the conflicting
 * Binary Inputs.
 *
 * State Change Event generation over Binary Inputs with group conflicts can
 * be obtained by this group, by deactivating the other group, and
 * calling this function for this group again.
 */
C_FUNC osStatus_t osBiGroupActivate(
    osBiGroupId biGroup /**< [in] The group to be activated */
  );

/**
 * \ingroup cmsis_os_ext_bi
 * \brief This deactivates the State Change Event generation for all
 * Binary Inputs that belong to a group.
 *
 * \note Works also in a Non RTOS environment.
 *
 * Deactivating a group will allow other Binary Input groups to take
 * over State Change Event generation for Binary Inputs that are member of the
 * deactivated group. See osBiGroupActivate(osBiGroupId).
 *
 * osMailGet(osMailQId queue_id, uint32_t) will not receive
 * any events anymore after this function returns.
 *
 * \return osOK, if successful, otherwise osErrorOS.
 */
C_FUNC osStatus_t osBiGroupDeativate(
    osBiGroupId biGroup /**< [in] The group to be deactivated. */
  );

/**
 * \ingroup cmsis_os_ext_bi
 * \brief Will delete a Binary Input group. De-activation will automatically
 * take place first.
 *
 * \note Works also in a Non RTOS environment.
 *
 */
C_FUNC osStatus_t osBiGroupDelete(
  osBiGroupId biGroup /**< [in] The Binary Input group to be deleted. */
  );


/**
 * \ingroup cmsis_os_ext_bi
 * \brief Retrieve the mail queue that will receive State Change Events for a Binary Input Group.
 *
 * \note Works also in a Non RTOS environment.
 *
 * \warning You must not delete the Binary Input group after you obtained its mail queue.
 *   The mail queue is a member of the Binary Input group object.
 * \return The mail queue receiving State Change Events for the Binary Input Group.
 */
C_FUNC osMessageQueueId_t osBiGroupGetMailQ(
  osBiGroupId biGroup /**< [in] The Binary Input group for which to obtain the mail queue */
  );

/**
 * \ingroup cmsis_os_ext_bi
 * \brief Retrieves the first Binary Input state change from an State Change Event mail, that
 * is returned by an osMailGet(osMailQId queue_id, uint32_t) call, where the queue_id is one that
 * must be obtained by osBiGroupGetMailQ(osBiGroupId biGroup).
 *
 * \note Works also in a Non RTOS environment.
 *
 * \return The first Binary Input Channel with a state change, in case there is one.
 * Otherwise bapi_bi_E_Invalid.
 */
C_FUNC bapi_E_BiChannel osBiGetFirstStateChange(
    const osBiStateChangeEvents* event               /**< [in]  The Event received from an osMailQ that was obtained from osBiGroupGetMailQ(osBiGroupId) */
  , bool* state                        /**< [out] The current state of the Binary Input. */
  );
/**
 * \ingroup cmsis_os_ext_bi
 * \brief Retrieves the next Binary Input state change from an State Change Event mail, that
 * is returned by an osMailGet(osMailQId queue_id, uint32_t) call, where the queue_id is one that
 * must be obtained by osBiGroupGetMailQ(osBiGroupId biGroup).
 *
 * \note Works also in a Non RTOS environment.
 *
 * \return The Binary Input with a state change that follows the a predecessor,
 * in case there is such a following Binary Input. Otherwise bapi_bi_E_Invalid.
 */
C_FUNC bapi_E_BiChannel osBiGetNextStateChange(
     const osBiStateChangeEvents* event               /**< [in]  The Event received from an osMailQ that was obtained from osBiGroupGetMailQ(osBiGroupId) */
   , bool* state                        /**< [out] The current state of the Binary Input. */
   , bapi_E_BiChannel predecessor       /**< [in]  Marks the start for searching the next Binary Input. */
   );

C_FUNC uint64_t osBiGetPulseCounter(const osBiStateChangeEvents* stateChangeEvents, bapi_E_BiChannel channel);
C_FUNC void osResetBiPulseCounter(osBiGroupId biGroup,bapi_E_BiChannel channel);
C_FUNC void osSetBiPulseCounter(osBiGroupId biGroup,bapi_E_BiChannel channel, uint64_t count);


#endif /* osIoBi_H_ */
