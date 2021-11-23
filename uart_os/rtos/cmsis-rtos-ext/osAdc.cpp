/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

/**
 * \file
 * \brief
 * This file defines the ARM CMSIS RTOS ADC Extension API.
 */

#include "baseplate.h"

#include <stdlib.h>
#include <new>

#include "osAdc.h"
#include "rtos/c++/osMailQueue.hpp"

uint16_t g_AllChannelAdcRawValue[bapi_adc_E_Ch_Count] = {0};

typedef osMessageQueueId_t AdcMailQueue_t;

typedef struct osAdcGroupShort_ {
  adcFreqDivider_t m_sampleFrequencyDivider;  /**< Divide the main ADC sample frequency for this group */
  adcBatchSize_t m_sampleBatchSize;         /**< How many samples to take, before all a notification takes place */
  uint16_t m_groupMemberCount;        /**< Number of group members following. */
  bapi_E_AdcChannel m_groupMembers[bapi_adc_E_Ch_Count];/**< Group member ADC channel. */
} osAdcGroupShort;

typedef struct osAdcGroup_ {
  AdcMailQueue_t   m_mailQ;
  adcFreqDivider_t m_sampleFrequencyDivider;  /**< Divide the main ADC sample frequency for this group. */
  adcBatchSize_t m_sampleBatchSize;         /**< How many samples to take, before all a notification takes place. */
  adcGroupMemberIndex_t m_groupMemberCount;        /**< Number of group members following. */
  _osAdcSampleBatch m_groupMembers[1]; /**< Group members. */
} osAdcGroup;

C_INLINE void adcGroup2adcGroupShort(struct osAdcGroupShort_*dst, const osAdcGroup* src) {
  dst->m_sampleFrequencyDivider = src->m_sampleFrequencyDivider;
  dst->m_sampleBatchSize = src->m_sampleBatchSize;
  dst->m_groupMemberCount = src->m_groupMemberCount;
  int m = src->m_groupMemberCount;

#ifdef _DEBUG
  size_t sampleBatchOffs  = OFFSETOF(_osAdcSampleBatch, m_sampleBatch);
#else
  #define sampleBatchOffs   OFFSETOF(_osAdcSampleBatch, m_sampleBatch);
#endif

  size_t srcGroupMemberSize  = (sizeof(adcSample_t) * src->m_sampleBatchSize) + sampleBatchOffs;

  while(m > 0) {
    --m;
    const _osAdcSampleBatch* srcGroupMember =
      R_CAST(const _osAdcSampleBatch*, (R_CAST(const uint8_t*, src->m_groupMembers) + m * srcGroupMemberSize));
    dst->m_groupMembers[m] = srcGroupMember->m_adcChannel;
  }
}



class AdcGroupManager {

  static const bapi_adc_ISRCallback_t INVALID_OLD_ISR_CALLBACK;

  typedef uint16_t adcGroupIndex_t;

  struct GroupMemberAddress{
    adcGroupMemberIndex_t m_memberIndex;
    adcGroupIndex_t       m_groupIndex;

    inline bool isValidGroup(adcGroupIndex_t activeAdcGroupsArraySize)const {
      return m_groupIndex < activeAdcGroupsArraySize;
    }

    GroupMemberAddress()
      : m_memberIndex(~S_CAST(adcGroupIndex_t, 0))
      , m_groupIndex (~S_CAST(adcGroupIndex_t, 0)) {
    }
  };

  bapi_adc_ISRCallback_t       m_oldISRCallback;  /**< The old callback function before we hooked in */
  adcFreqDivider_t          m_conversionCounter;  /**< Counts the number of ADC conversions to manage the
                                                       frequency dividers of the groups. */

  adcGroupIndex_t    m_activeAdcGroupsArraySize;  /**< The index of the most upper used item in. */
  osAdcGroup* m_activeAdcGroups[osMaxAdcGroups];  /**< The active ADC groups, that do frequent conversion. */

  /**
   * For each ADC channel a shortcut to the group member.
   */
  GroupMemberAddress m_groupMemberAdresses[bapi_adc_E_Ch_Count];

  bool __onAdcConversionComplete(
    const struct GroupMemberAddress& groupMemberAddress,
    uint16_t adcRawValue,
    bapi_E_AdcChannel adcChannel
    ) {

	  g_AllChannelAdcRawValue[adcChannel] = adcRawValue;

    if (groupMemberAddress.isValidGroup(m_activeAdcGroupsArraySize)) {
      osAdcGroup* adcGroup = m_activeAdcGroups[groupMemberAddress.m_groupIndex];
      if (adcGroup) {
#ifdef _DEBUG
        size_t sampleBatchOffs = OFFSETOF(_osAdcSampleBatch, m_sampleBatch);
#else
#define sampleBatchOffs   OFFSETOF(_osAdcSampleBatch, m_sampleBatch);
#endif
        size_t groupMemberSize = (sizeof(adcSample_t) * adcGroup->m_sampleBatchSize) + sampleBatchOffs;

        _osAdcSampleBatch* adcGroupMember =
          R_CAST(_osAdcSampleBatch*,
            (R_CAST(uint8_t*, adcGroup->m_groupMembers) + groupMemberAddress.m_memberIndex * groupMemberSize));

        if (adcGroupMember->m_adcChannel == adcChannel) {

          adcGroupMember->m_sampleBatch[adcGroupMember->m_sampleCounter] = adcRawValue;
          ++adcGroupMember->m_sampleCounter;
          if (adcGroupMember->m_sampleCounter >= adcGroup->m_sampleBatchSize) {

            /* sample batch is full -> notify */
            //void* mail = osMailAlloc(adcGroup->m_mailQ, 0);

            //if (mail) {
            //  memcpy(mail, adcGroupMember, groupMemberSize);
            //  osStatus result = osMailPut(adcGroup->m_mailQ, mail);
            //  if (result != osOK) {
            //    osMailFree(adcGroup->m_mailQ, mail);
            //  }
            //}

            osMessageQueuePut(adcGroup->m_mailQ, adcGroupMember, 0, 0 );

            adcGroupMember->m_sampleCounter %= adcGroup->m_sampleBatchSize;
          }
          return true;
        }
      }
    }
    return false;
  }

  inline struct GroupMemberAddress findAdcChannel(bapi_E_AdcChannel adcChannel){
    ASSERT(bapi_irq_isInterruptContext()); /* must only be called by within the ADC callback context,
                                    which allows us to let interrupts enabled. */
    struct GroupMemberAddress retval;
    for(retval.m_groupIndex = 0; retval.m_groupIndex < m_activeAdcGroupsArraySize; retval.m_groupIndex++) {
      if(m_activeAdcGroups[retval.m_groupIndex]) {
        const osAdcGroup* adcGroup = m_activeAdcGroups[retval.m_groupIndex];

#ifdef _DEBUG
        size_t sampleBatchOffs  = OFFSETOF(_osAdcSampleBatch, m_sampleBatch);
#else
        #define sampleBatchOffs   OFFSETOF(_osAdcSampleBatch, m_sampleBatch);
#endif
        size_t groupMemberSize  = (sizeof(adcSample_t) * adcGroup->m_sampleBatchSize) + sampleBatchOffs;

        for(retval.m_memberIndex = 0; retval.m_memberIndex < adcGroup->m_groupMemberCount; retval.m_memberIndex++) {
          const _osAdcSampleBatch* srcGroupMember =
            R_CAST(const _osAdcSampleBatch*, (R_CAST(const uint8_t*, adcGroup->m_groupMembers) + retval.m_memberIndex * groupMemberSize));
          if(srcGroupMember->m_adcChannel == adcChannel) {
            return retval;
          }
        }
      }
    }

    return retval;
  }

  inline void _onAdcConversionComplete(bapi_E_AdcChannel adcChannel, uint16_t adcRawValue) {
    ASSERT(bapi_irq_isInterruptContext()); /* must only be called by within the ADC callback context only. */

    if(!__onAdcConversionComplete(m_groupMemberAdresses[adcChannel], adcRawValue, adcChannel)) {

      /* The shortcut was not be initialized correctly, so we do now. */
      m_groupMemberAdresses[adcChannel] = findAdcChannel(adcChannel);
      __onAdcConversionComplete(m_groupMemberAdresses[adcChannel], adcRawValue, adcChannel);
    }

    if(m_oldISRCallback) {
      (*m_oldISRCallback)(adcChannel, adcRawValue);
    }
  }

  static void onAdcConversionComplete(bapi_E_AdcChannel adcChannel, uint16_t adcRawValue) {
    theGroupManager._onAdcConversionComplete(adcChannel, adcRawValue);
  }


  inline void disable() {
    bapi_adc_setConversionComplete_ISRCallback(m_oldISRCallback);
    m_oldISRCallback = INVALID_OLD_ISR_CALLBACK;
  }

  inline void enable() {
    if(m_oldISRCallback == INVALID_OLD_ISR_CALLBACK) {
      m_oldISRCallback = bapi_adc_setConversionComplete_ISRCallback(onAdcConversionComplete);
    }
  }


  AdcGroupManager()
    : m_oldISRCallback(INVALID_OLD_ISR_CALLBACK)
    , m_activeAdcGroupsArraySize(0)
    , m_conversionCounter(0) {
  }

  int findAdcGroup(osAdcGroup* adcGroup) {
    int g = m_activeAdcGroupsArraySize;
    while(g > 0) {
      --g;
      if(m_activeAdcGroups[g] == adcGroup) {
        return g;
      }
    }
    return -1;
  }
  
public:
  /** The one and only instance of the Group Manager */
  static AdcGroupManager theGroupManager;

  /* It is inline because there is only one caller. */
  inline bool addGroup(osAdcGroup* adcGroup) {

    /* The algorithm here is optimized in order to have IRQs disabled as short as possible. */
    /* find an empty entry */
    int g = findAdcGroup(0);

    while(g >= 0) {
      bapi_irq_enterCritical();
      if(m_activeAdcGroups[g] == 0) {
        m_activeAdcGroups[g] = adcGroup;
        bapi_irq_exitCritical();
        return true;
      } else {
        /* A nested call of addGroup happened, so retry. */
        bapi_irq_exitCritical();
        /* find an empty entry */
        g = findAdcGroup(0);
      }
    }

    /* We couldn't find a free entry, so try to enlarge the array. */
    bapi_irq_enterCritical();
    if(m_activeAdcGroupsArraySize < ARRAY_SIZE(m_activeAdcGroups)) {
      m_activeAdcGroups[m_activeAdcGroupsArraySize] = adcGroup;
      if(!m_activeAdcGroupsArraySize) {
        enable();
      }
      ++m_activeAdcGroupsArraySize;
      bapi_irq_exitCritical();
      return true;
    }
    bapi_irq_exitCritical();
    return false;
  }

  /* It is inline because there is only one caller. */
  inline void removeGroup(osAdcGroup* adcGroup) {
    /* The algorithm here is optimized in order to have IRQs disabled as short as possible. */
    /* find the group */
    int g = findAdcGroup(adcGroup);
    while(g >= 0) {
      bapi_irq_enterCritical();
      if(m_activeAdcGroups[g] == adcGroup) {
        m_activeAdcGroups[g] = 0;

        /* if the removed one was the last one, shrink the array. */
        if( (g + 1) == m_activeAdcGroupsArraySize) {
          --m_activeAdcGroupsArraySize;
          if(!m_activeAdcGroupsArraySize) {
            disable();
          }
        }
        bapi_irq_exitCritical();
        return;
      } else {

        /* A nested call of removeGroup happened, so retry. */
        bapi_irq_exitCritical();
        /* find the group */
        g = findAdcGroup(adcGroup);
      }
    }
    return;
  }

  /* It is inline because there is only one caller. */
  inline void startAdcConversion() {
    atomic_Increment(&m_conversionCounter);

    size_t g = m_activeAdcGroupsArraySize;
    /* Process all groups in reverse order. Hence a group that will
     * be asynchronously added here will be considered in the next
     * startAdcConversion() call. */

    bapi_irq_enterCritical();
    while(g > 0) {
      --g;
      const osAdcGroup* currentGroup = m_activeAdcGroups[g];

      if(m_activeAdcGroups[g]) {
        osAdcGroupShort adcGroup;
        adcGroup2adcGroupShort(&adcGroup, currentGroup);
        bapi_irq_exitCritical();

        /* Skip conversion based on sample frequency divider of this group */
        if((m_conversionCounter % adcGroup.m_sampleFrequencyDivider) == 0) {
          /* Start conversion for each ADC channel of this group */
          bapi_adc_startAdcConversionByChannelList(adcGroup.m_groupMembers, adcGroup.m_groupMemberCount);
        }
      } else {
        bapi_irq_exitCritical();
      }
      bapi_irq_enterCritical();
    }
    bapi_irq_exitCritical();
  }
};

const bapi_adc_ISRCallback_t AdcGroupManager::INVALID_OLD_ISR_CALLBACK = AdcGroupManager::onAdcConversionComplete;


AdcGroupManager AdcGroupManager::theGroupManager; /**< The one and only instance of the Group Manager */

osMessageQueueId_t osAdcMailQCreate(unsigned int adcMailQueueSize, adcBatchSize_t sampleBatchSize) {
#ifdef _DEBUG
  size_t sampleBatchOffs  = OFFSETOF(_osAdcSampleBatch, m_sampleBatch);
#else
  #define sampleBatchOffs   OFFSETOF(_osAdcSampleBatch, m_sampleBatch);
#endif
  size_t groupMemberSize  = (sizeof(adcSample_t) * sampleBatchSize) + sampleBatchOffs;
  //osMailQDef_t mailQdef;
  //mailQdef.queue_sz =  MAX(1, adcMailQueueSize);
  //mailQdef.item_sz = groupMemberSize;
  //mailQdef.name = 0;
  //return osMailCreate(&mailQdef, 0);

  const osMessageQueueAttr_t mssgQAtt = {NULL, 0, NULL, 0, NULL, 0};
  return osMessageQueueNew(MAX(1, adcMailQueueSize), groupMemberSize, &mssgQAtt);
}

osAdcGroupId osAdcGroupCreate(const bapi_E_AdcChannel* groupMembers
  , adcGroupMemberIndex_t groupMemberCount
  , adcFreqDivider_t sampleFrequencyDivider
  , adcBatchSize_t sampleBatchSize
  ) {
  ASSERT(sampleFrequencyDivider != 0);
  ASSERT(sampleBatchSize > 0);
  ASSERT(!bapi_irq_isInterruptContext()); /* Not allowed in ISR context */

#ifdef _DEBUG
  size_t sampleBatchOffs  = OFFSETOF(_osAdcSampleBatch, m_sampleBatch);
  size_t groupMembersOffs = OFFSETOF(osAdcGroup, m_groupMembers);
#else
  #define sampleBatchOffs   OFFSETOF(_osAdcSampleBatch, m_sampleBatch);
  #define groupMembersOffs  OFFSETOF(osAdcGroup, m_groupMembers);
#endif

  size_t groupMemberSize  = (sizeof(adcSample_t) * sampleBatchSize) + sampleBatchOffs;
  
  //Making sure Member Count is within supported range
  if(groupMemberCount > bapi_adc_E_Ch_Count) {
    groupMemberCount = bapi_adc_E_Ch_Count;
  }
  //Groupsize calculation now takes into account MAX channel possible instead of dynamically assigned
  // number of channels. This calculation is changed only after a weird memory corruption related crash
  // observed.
  //TODO Rewrite this complicated ADC grouping code make it simple and easy to understand.
  size_t groupSize = groupMemberSize * /*groupMemberCount*/bapi_adc_E_Ch_Count + groupMembersOffs;

  osAdcGroupId retval = S_CAST(osAdcGroup*, malloc(groupSize));
  if(retval) {
    memset(retval, 0, groupSize);

    retval->m_sampleFrequencyDivider = sampleFrequencyDivider;
    retval->m_sampleBatchSize = sampleBatchSize;
    retval->m_groupMemberCount = groupMemberCount;
    _osAdcSampleBatch* groupMember = retval->m_groupMembers;
    while(groupMemberCount) {
      groupMember->m_adcChannel = *groupMembers++;
      groupMember = R_CAST(_osAdcSampleBatch*, (R_CAST(uint8_t*, groupMember) + groupMemberSize));
      --groupMemberCount;
    }

  }
  return retval;
}

/**
 * \ingroup cmsis_os_ext_adc
 */
C_FUNC void osAdcConversionStart() {
  AdcGroupManager::theGroupManager.startAdcConversion();
}

osStatus_t osAdcGroupActivate(osAdcGroupId adcGroup, osMessageQueueId_t queue_id) {
  ASSERT(!bapi_irq_isInterruptContext()); /* Not allowed in ISR context. */
  ASSERT(queue_id);             /* Assert a nonzero mail queue.*/

#ifdef _DEBUG
  size_t sampleBatchOffs  = OFFSETOF(_osAdcSampleBatch, m_sampleBatch);
#else
  #define sampleBatchOffs   OFFSETOF(_osAdcSampleBatch, m_sampleBatch);
#endif
  size_t groupMemberSize  = (sizeof(adcSample_t) * adcGroup->m_sampleBatchSize) + sampleBatchOffs;

  osStatus_t retval = osErrorParameter;

  bapi_irq_enterCritical();

  /* See if an ADC update mail for this ADC group would fit. */
  //if(osMailBlockSizeGet_suppl(queue_id) >= groupMemberSize) {
  if(osMessageQueueGetMsgSize(queue_id) >= groupMemberSize) {

    if(!adcGroup->m_mailQ) {
      adcGroup->m_mailQ = queue_id;

      bapi_irq_exitCritical();

      if(AdcGroupManager::theGroupManager.addGroup(adcGroup)) {
        retval = osOK;
      } else {
        adcGroup->m_mailQ = 0;
        retval = osError;
      }

      /* return here, in order to not call bapi_irq_exitCritical() */
      return retval;
    }

  }

  bapi_irq_exitCritical();
  return retval;
}

osStatus_t osAdcGroupDecativate(osAdcGroupId adcGroup) {
  ASSERT(!bapi_irq_isInterruptContext()); /* Not allowed in ISR context */
  AdcGroupManager::theGroupManager.removeGroup(adcGroup);
  adcGroup->m_mailQ = 0; /* Assignment is atomic because mailQ is a pointer. */
  return osOK;
}

osStatus_t osAdcGroupDelete(osAdcGroupId adcGroup) {
  ASSERT(!bapi_irq_isInterruptContext()); /* Not allowed in ISR context */
  osStatus_t retval = osAdcGroupDecativate(adcGroup);
  if(retval == osOK) {
//    /* Call destructor of mailQ explicitly because it was created with
//     * placement new within biGroup allocated memory. */
//    osMailDestroy_suppl( &adcGroup->m_mailQ, osWaitForever ); // TODO: pass timeout from additional parameter of this function.
    free(adcGroup);
  }

  return retval;
}

int32_t osAdcAverage(const _osAdcSampleBatch_& updateEvent)
  {
  /* We are interested in the average, so add up each sample from the batch. */
  int32_t sum = 0; /* The sample can be 16 bit wide, we need to provide more bits for building the sum. */
  const adcSample_t* adcSample = osAdcUpdateGetFirst(&updateEvent);

  while ( adcSample ) {
    sum += *adcSample;
    adcSample = osAdcUpdateGetNext(&updateEvent, adcSample);
  }

  adcBatchSize_t batchSize = osAdcUpdateBatchSizeGet(&updateEvent);
  const int32_t average = sum / batchSize;
  return average;
}
