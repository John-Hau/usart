/*
 * osCom.h
 *
 *  Created on: 07.04.2013
 *      Author: Wolfgang
 */

#ifndef osIoAdc_H_
#define osIoAdc_H_


#include "baseplate.h"
#include "boards/board-api/bapi_uart.h"
#include "cmsis-driver/Driver_USART.h"

#ifdef __IAR_SYSTEMS_ICC__
  #include <LowLevelIOInterface.h>
#else
  #include <unistd.h>
#endif

#include "boards/board-api/bapi_io.h"
#include "rtos/cmsis-rtos/cmsis_os_redirect.h"

#ifndef osMaxAdcGroups
  #define osMaxAdcGroups 3
#endif

/**
 * \file
 * This file declares the ARM CMSIS RTOS ADC Extension API.
 */

struct osAdcGroup_;
typedef struct osAdcGroup_* osAdcGroupId;
struct _osAdcSampleBatch_;

typedef uint16_t adcFreqDivider_t;
typedef uint16_t adcBatchSize_t;
typedef uint16_t adcGroupMemberIndex_t;
typedef uint16_t adcSample_t;

typedef adcBatchSize_t _adcSampleCounter_t;

/**
 * \ingroup _cmsis_os_ext_adc
 * A structure that stores an ADC channel along with the its sample batch state,
 * which is the aggregation of the m_sampleCounter as well as m_sampleBatch.
 *
 */
typedef struct _osAdcSampleBatch_ {
  bapi_E_AdcChannel   m_adcChannel;        /**< The ADC channel to which this sample batch belongs. */
  _adcSampleCounter_t m_sampleCounter;     /**< Currently completed samples for the sample batch.  */
  adcSample_t         m_sampleBatch[1];    /**< The batch of ADC samples. The array size is bigger
                                               than one as statically declared. The size is according
                                            *  to what has been passed as sampleBatchSize parameter
                                            *  in function osAdcGroupCreate(
                                            *     const bapi_E_AdcChannel* groupMembers
                                            *   , adcGroupMemberIndex_t groupMemberCount
                                            *   , adcFreqDivider_t sampleFrequencyDivider
                                            *   , adcBatchSize_t sampleBatchSize
                                            *   )
                                            */
} _osAdcSampleBatch;

extern uint16_t g_AllChannelAdcRawValue[bapi_adc_E_Ch_Count];
/**
 * \ingroup cmsis_os_ext_adc
 * \brief Creates a group of ADC Inputs. ADC Value Update mails will be
 * generated for all active ADC Inputs in the group.
 *
 * \note Works also in a Non RTOS environment.
 *
 * ADC Input Channel groups allow you to treat particular sets of ADC Inputs differently.
 * ADC Input Channels of the same group have a common behavior in terms of how often
 * samples are taken (i.e. ADC conversions are triggered) and how many
 * samples are collected in a sample batch before this batch is send to a mail queue.
 *
 * A thread can wait for ADC Value Update mails of one or multiple ADC groups by calling
 * osMailGet(osMailQId queue_id, uint32_t), where queue_id was created by
 * osAdcMailQCreate(unsigned int , adcBatchSize_t) and has been passed to
 * osAdcGroupActivate(osAdcGroupId adcGroup, osMailQId queue_id).
 *
 * A particular ADC Input can only be associated with one
 * group. If an ADC Input is put in multiple groups, it will
 * only send ADC Value updates to the mail queue of the group that was activated first.
 *
 * - __Frequency divider__:
 *  Assume the function osAdcConversionStart() is called by an RTOS timer or hardware
 *  timer each 3 milliseconds. In case that the sampleFrequencyDivider for a group is
 *  set to 5, samples for all members of that group will be taken every 3ms x 5 = 15ms.
 *
 * - __Sample Batch__:
 *  Assume in the above example the group has a batch size of 8, a mail notification
 *  will be sent to a waiting thread every 15ms x 8 = 120ms. This mail will contain the
 *  most recent 8 samples that have been taken with a time distance of 15ms.
 *
 * - __Mail queue__:
 *  Assume that you have a second group with another batch size and a different
 *  frequency divider. You can have a single thread waiting for all notification of both
 *  groups by activating the groups with the same mail queue. Refer to
 *  osAdcGroupActivate(osAdcGroupId adcGroup, osMailQId queue_id). In that case the
 *  mail queue must have been created to carry mails with the larger batch size of
 *  both groups.
 *  Alternatively you can have 2 individual threads waiting for a group dedicated
 *  mail queue. In that case each mail queue shall be created with the same batch size
 *  as the dedicated group.
 *
 *
 */
C_FUNC osAdcGroupId osAdcGroupCreate(
    const bapi_E_AdcChannel* groupMembers   /**< [in] The ADC Input Channels that belong to the new group */
  , adcGroupMemberIndex_t groupMemberCount  /**< [in] The size of the groupMembers array */
  , adcFreqDivider_t sampleFrequencyDivider /**< [in] The frequency divider for taking samples */
  , adcBatchSize_t sampleBatchSize          /**< [in] The size of the sample batch */
  );

/**
 * \ingroup cmsis_os_ext_adc
 * \brief Creates a mail queue that can be used to receive ADC Value Update mails.
 *
 * \note Works also in a Non RTOS environment.
 *
 * This mail queue will receive mails from all ADC Inputs that are in a group that
 * has been passed to the function.
 * osAdcGroupActivate(osAdcGroupId adcGroup, osMailQId queue_id).
 *
 * \warning Do never delete a mail queue that is still active for an ADC group. E.g. if
 * you have activated a mail queue for 3 different groups you must deactivate them first
 * before deleting the mail queue. Refer to osAdcGroupDecativate(osAdcGroupId adcGroup).
 */
C_FUNC osMessageQueueId_t osAdcMailQCreate(
    unsigned int adcMailQueueSize   /**< The number of entries in the ADC Value Update mail queue. */
  , adcBatchSize_t sampleBatchSize  /**< The number of batch samples that fit into one ADC Value Update mail. */
  );

/**
 * \ingroup cmsis_os_ext_adc
 * \brief will start an ADC conversion for all ADC channels that are in any active ADC group.
 *
 * \note Works also in a Non RTOS environment.
 *
 * The calling frequency of this function will decide the basic sample frequency. ADC groups
 * may divide this frequency further.
 *
 * \sa osAdcGroupCreate(const bapi_E_AdcChannel* groupMembers
 *   , adcGroupMemberIndex_t groupMemberCount
 *   , adcFreqDivider_t sampleFrequencyDivider
 *   , adcBatchSize_t sampleBatchSize
 *   )
 */
C_FUNC void osAdcConversionStart();

/**
 * \ingroup cmsis_os_ext_adc
 * \brief Activates an ADC group to collect samples to be placed into a sample batch.
 *
 * \note Works also in a Non RTOS environment.
 *
 * ADC Value Update mails will be generated whenever the sample batch is full.
 *
 * \sa osAdcGroupCreate(const bapi_E_AdcChannel* groupMembers
 *   , adcGroupMemberIndex_t groupMemberCount
 *   , adcFreqDivider_t sampleFrequencyDivider
 *   , adcBatchSize_t sampleBatchSize
 *   )
 *
 */
C_FUNC osStatus_t osAdcGroupActivate(
      osAdcGroupId adcGroup /**< [in] The ADC group to be activated. */
    , osMessageQueueId_t queue_id    /**< [in] The mail queue that shall receive ADC Value Update mails from the group. */
  );

/**
 * \ingroup cmsis_os_ext_adc
 * \brief Deactivates an ADC group. No ADC samples will be collected anymore and no ADC Value Update mails
 * will be generated anymore.
 * \note Works also in a Non RTOS environment.
 *
 */
C_FUNC osStatus_t osAdcGroupDecativate(
  osAdcGroupId adcGroup /**< [in] The ADC group to be de-activated. */
  );

/**
 * \ingroup cmsis_os_ext_adc
 * \brief Will delete an ADC Input group. De-activation will automatically
 * take place first.
 *
 * \note Works also in a Non RTOS environment.
 *
 */
C_FUNC osStatus_t osAdcGroupDelete(
  osAdcGroupId adcGroup /**< [in] The ADC group to be deleted. */
  );


/**
 * \ingroup cmsis_os_ext_adc
 * \brief Dispatches an ADC Value Update mail from an ADC mail queue to return the ADC
 * channel that is associated with this mail.
 *
 * \note Works also in a Non RTOS environment.
 *
 * \return the ADC channel that belongs to the sample batch within the ADC Value Update mail,
 * if the passed osEvent contains a valid mail. Otherwise bapi_adc_E_Ch_INVALID.
 *
 */
C_INLINE bapi_E_AdcChannel osAdcUpdateChannelGet(
  //const osEvent* osEvent /**< [in] The osEvent that was received from the osMailGet(osMailQId queue_id, uint32_t) call */
  const _osAdcSampleBatch_* updateEvent
  ) {
  //if(osEvent->status == osEventMail) {
  if(updateEvent) {
    //const _osAdcSampleBatch_* updateEvent = S_CAST(const _osAdcSampleBatch_*, osEvent->value.p);
    return updateEvent->m_adcChannel;
  }
  return bapi_adc_E_Ch_INVALID;
}

/**
 * \ingroup cmsis_os_ext_adc
 * \brief Dispatches an ADC Value Update mail from an ADC mail queue to return the
 * sample batch size that is associated with this mail.
 *
 * \note Works also in a Non RTOS environment.
 *
 * \return The sample batch size if the passed osEvent contains a valid mail. Otherwise null.
 */
C_INLINE adcBatchSize_t osAdcUpdateBatchSizeGet(
  //const osEvent* osEvent /**< [in] The osEvent that was received frome the osMailGet(osMailQId queue_id, uint32_t) call */
  const _osAdcSampleBatch_* updateEvent
  ) {
  //if(osEvent->status == osEventMail) {
  if(updateEvent) {
    //const _osAdcSampleBatch_* updateEvent = S_CAST(const _osAdcSampleBatch_*, osEvent->value.p);
    return updateEvent->m_sampleCounter;
  }
  return 0;
}

/**
 * \ingroup cmsis_os_ext_adc
 * \brief Dispatches an ADC Value Update mail from an ADC mail queue in order to
 * retrieves the first sample from the sample batch within the mail.
 *
 * \note Works also in a Non RTOS environment.
 *
 * \return The first sample, in case there is one. Otherwise null.
 */
C_INLINE const adcSample_t* osAdcUpdateGetFirst(
  //const osEvent* osEvent /**< [in] The osEvent that was received frome the osMailGet(osMailQId queue_id, uint32_t) call */
  const _osAdcSampleBatch_* updateEvent
  ) {
  //if(osEvent->status == osEventMail) {
  if(updateEvent) {
    //const _osAdcSampleBatch_* updateEvent = S_CAST(const _osAdcSampleBatch_*, osEvent->value.p);
    ASSERT(updateEvent->m_sampleCounter);
    return &updateEvent->m_sampleBatch[0];
  }
  return 0;
}

/**
 * \ingroup cmsis_os_ext_adc
 * \brief Dispatches an ADC Value Update mail from an ADC mail queue in order to
 * retrieves the first sample from the sample batch within the mail.
 *
 * \note Works also in a Non RTOS environment.
 *
 * \return The sample that follows the a predecessor, in case there is one. Otherwise null.
 */
C_INLINE const adcSample_t* osAdcUpdateGetNext(
  //const osEvent* osEvent           /**< [in] The osEvent that was received frome the osMailGet(osMailQId queue_id, uint32_t) call */
  const _osAdcSampleBatch_* updateEvent
  , const adcSample_t* predecessor /**< [in]  Marks the start for searching the next sample. */
  ) {
  //if(osEvent->status == osEventMail) {
  if(updateEvent) {
    //const _osAdcSampleBatch_* updateEvent = S_CAST(const _osAdcSampleBatch_*, osEvent->value.p);
    const adcSample_t* end = &updateEvent->m_sampleBatch[updateEvent->m_sampleCounter];
    if(++predecessor < end) {
      return predecessor;
    }
  }
  return 0;
}

/**
 * \ingroup cmsis_os_ext_adc
 * \brief Calculates the average from an ADC Value Update mail from an ADC mail queue
 *
 * \note Works also in a Non RTOS environment.
 *
 * \return the Calculated average from sample batch that is associated with ADC Value
 *   Update mail.
 *
 */
C_FUNC int32_t osAdcAverage(const _osAdcSampleBatch_& updateEvent);

#endif /* osIoAdc_H_ */
