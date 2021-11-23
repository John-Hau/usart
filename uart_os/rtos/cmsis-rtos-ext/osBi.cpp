/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

/**
 * \file
 * \brief
 * This file defines the ARM CMSIS RTOS Binary Input Extension API.
 */

#include "baseplate.h"
#include <string.h>

#include <stdlib.h>
#include <stddef.h>
#include <new>

#include "osBi.h"
#include "rtos/c++/osMailQueue.hpp"
#include "boards/board-api/bapi_io.h"
#ifdef _DEBUG
//  #define BIGROUP_DEBUG
#endif

#ifdef BIGROUP_DEBUG
  enum bigroup_E_MailTag{ BIGROUP_MAILTAG = 0x10091960 };
#endif

#if 0
typedef uint32_t bitfield_base_t;

enum {
   BI_MAIL_Q_SIZE = 2
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
#endif /* #ifdef __cplusplus */

} _osBiGroupMembers;
#endif //#if 0

// typedef os::MailQueue<_osBiGroupMembers> BiMailQueue_t;

typedef struct osBiGroup_ {
  osMessageQueueId_t m_messageQ;
  _osBiGroupMembers m_groupMembers;

  void onStateChange(bapi_E_BiChannel biChannel, bool state) {

    ASSERT(bapi_irq_isInterruptContext()); /* This must only be called from the ISR */

    /** See if we need to update the info of a pending mail */

    _osBiGroupMembers front;

    if( osOK != osMessageQueueGet(m_messageQ, &front, NULL, 0) ){
//      memset(&front, sizeof(front), 0); //stupid mistake
	  memset(&front, 0, sizeof(front));
    }

    /* The mail Q is 2 entries, so there must be a free one if BI irq nesting is disabled. */
    ASSERT(bapi_bi_getBiIrqNesting() == false);
    //ASSERT(front);

#ifdef BIGROUP_DEBUG
    //front->m_mailTag = BIGROUP_MAILTAG;
    front.m_mailTag = BIGROUP_MAILTAG;
#endif

    if(state) {
      //front->setBiChannelState(biChannel);
      front.setBiChannelState(biChannel);
    } else {
      //front->resetBiChannelState(biChannel);
      front.resetBiChannelState(biChannel);
    }
#if defined (FS_BEATS_IO)
	front.pulse_counter[biChannel] = m_groupMembers.pulse_counter[biChannel]; //h242608
#endif
    //osMailPut(m_mailQ, front);
    //osMessageQueuePut(m_messageQ, &front, 0, osWaitForever ); //TODO - is "osWaitForever" correct ?????
#if defined (FS_BEATS_IO)
    osMessageQueuePutOverWrite(m_messageQ, &front, 0, 0 );
#else
    osMessageQueuePut(m_messageQ, &front, 0, 0 );
#endif
  }

} osBiGroup;

#if (BAPI_HAS_BI_CHANNEL > 0)

class BiGroupManager {

  bapi_bi_stateChanged_ISRCallback_t m_oldISRCallback;/**< The old callback function before we hooked in */
  osBiGroup* m_biGroup[bapi_bi_E_Ch_Count];           /**< The associated group For each Binary Input */

  static void onBiStateChange(bapi_E_BiChannel biChannel, bool state) {
#if defined (FS_BEATS_IO)
theGroupManager.m_biGroup[biChannel]->m_groupMembers.pulse_counter[biChannel]++;
#endif
    theGroupManager._onBiStateChange(biChannel, state);
  }

  inline void _onBiStateChange(bapi_E_BiChannel biChannel, bool state) {
    if(m_biGroup[biChannel]) {
      m_biGroup[biChannel]->onStateChange(biChannel, state);
    }

    if(m_oldISRCallback) {
      (*m_oldISRCallback)(biChannel, state);
    }
  }

  inline bool _attachChannelToGroup(bapi_E_BiChannel biChannel, osBiGroup* biGroup) {
    /* Assert that this function is not called to remove a group. */
    ASSERT(biGroup);
    if(biGroup) {
      /* Assert that biChannel is not already in another group. */
      ASSERT((m_biGroup[biChannel] == 0) || (m_biGroup[biChannel] == biGroup));
      if(m_biGroup[biChannel] == 0) {
        m_biGroup[biChannel] = biGroup;
        bapi_bi_setInterruptMode(biChannel, 1, true, true);
        return true;
      } else if(m_biGroup[biChannel] == biGroup) {
        return true; /* Nothing has changed. */
      }
    }
    return false;
  }

  inline void _detachChannelFromGroup(bapi_E_BiChannel biChannel) {
    bapi_bi_setInterruptMode(biChannel, 1, false, false);
    m_biGroup[biChannel] = 0;
  }

  inline void disable() {
    bapi_bi_setStateChange_ISRCallback(m_oldISRCallback);
  }

  inline void enable() {
    if(m_oldISRCallback == onBiStateChange) {
      m_oldISRCallback = bapi_bi_setStateChange_ISRCallback(onBiStateChange);
    }
  }

  BiGroupManager() : m_oldISRCallback(onBiStateChange){
    size_t bi = 0;
    for(; bi < bapi_bi_E_Ch_Count; bi++) {
      m_biGroup[bi] = 0;
    }

    bapi_bi_disableBiIrqNesting();
  }

public:
  /** The one and only instance of the Group Manager */
  static BiGroupManager theGroupManager;

  /**
   * Attaches a Binary Input channel to a new group. A Channel can
   * only be in one group.
   */
  bool attachChannelToGroup(bapi_E_BiChannel biChannel, osBiGroup* biGroup) {
    bool retval = false;
    ASSERT(biGroup);

    bapi_irq_enterCritical();
    if(biGroup) {
      enable();
      retval = _attachChannelToGroup(biChannel, biGroup);
    }
    bapi_irq_exitCritical();
    return retval;
  }

  inline void detachChannelFromGroup(bapi_E_BiChannel biChannel) {
    bapi_irq_enterCritical();
    _detachChannelFromGroup(biChannel);
    bapi_irq_exitCritical();
  }
};

BiGroupManager BiGroupManager::theGroupManager; /**< The one and only instance of the Group Manager */

#endif /* (BAPI_HAS_BI_CHANNEL > 0) */


C_FUNC osBiGroupId osBiGroupCreate(const bapi_E_BiChannel* groupMembers
  , unsigned int groupMemberCount
  ) {
  ASSERT(!bapi_irq_isInterruptContext()); /* Not allowed in ISR context */
  osBiGroupId retval = S_CAST(osBiGroup*, malloc(sizeof(struct osBiGroup_)));
  if(retval) {
    {
      memset(retval, 0, sizeof(osBiGroup));
      while(groupMemberCount) {
        retval->m_groupMembers.addBiChannel(groupMembers[--groupMemberCount]);
      }
    }
    {
      //osMailQDef_t mailQDef;
      //mailQDef.item_sz = sizeof(_osBiGroupMembers);
      //mailQDef.queue_sz = BI_MAIL_Q_SIZE;
      //mailQDef.name = 0;
      //retval->m_mailQ = osMailCreate(&mailQDef,0);

      const osMessageQueueAttr_t mssgQAtt = {NULL, 0, NULL, 0, NULL, 0};
      retval->m_messageQ = osMessageQueueNew(BI_MAIL_Q_SIZE, sizeof(_osBiGroupMembers), &mssgQAtt);
    }
  }
  return retval;
}

C_FUNC osStatus_t osBiGroupActivate(osBiGroupId biGroup) {
  ASSERT(!bapi_irq_isInterruptContext()); /* Not allowed in ISR context */
  osStatus_t retval = osOK;
  size_t index = 0;
  for(; index < BITFIELD_ARRAY_SIZE; index++) {
    size_t bitNo = 0;
    for(; bitNo < BITS_PER_BYTE * sizeof(bitfield_base_t); bitNo++) {
      if( biGroup->m_groupMembers.m_biPresenceBitfield[index] & (1 << bitNo)) {
        int biChannel = sizeof(bitfield_base_t) * index + bitNo;
        if(biChannel < bapi_bi_E_Ch_Count) {
          if( !BiGroupManager::theGroupManager.attachChannelToGroup(S_CAST(bapi_E_BiChannel, biChannel), biGroup) ) {
            /* This Binary Input could not be attached to the group, because it belongs to another group already. */
            //retval = osErrorOS;
            retval = osError;
          }
        }
      }
    }
  }
  return retval;
}

osStatus_t osBiGroupDeativate(osBiGroupId biGroup) {
  ASSERT(!bapi_irq_isInterruptContext()); /* Not allowed in ISR context */
  size_t index = 0;
  for(; index < BITFIELD_ARRAY_SIZE; index++) {
    size_t bitNo = 0;
    for(; bitNo < BITS_PER_BYTE * sizeof(bitfield_base_t); bitNo++) {
      if( biGroup->m_groupMembers.m_biPresenceBitfield[index] & (1 << bitNo)) {
        int biChannel = sizeof(bitfield_base_t) * index + bitNo;
        if(biChannel < bapi_bi_E_Ch_Count) {
          BiGroupManager::theGroupManager.detachChannelFromGroup(S_CAST(bapi_E_BiChannel, biChannel));
        }
      }
    }
  }
  return osOK;
}

C_FUNC osStatus_t osBiGroupDelete(osBiGroupId biGroup) {
  ASSERT(!bapi_irq_isInterruptContext()); /* Not allowed in ISR context */
  osStatus_t retval = osBiGroupDeativate(biGroup);

  if (retval == osOK) {
    /* Call destructor of mailQ explicitly because it was created with
     * placement new within biGroup allocated memory. */
    //osMailDestroy_suppl(&biGroup->m_mailQ, osWaitForever); // TODO: pass timeout from additional parameter of this function.
    osMessageQueueDelete(biGroup->m_messageQ);
    free(biGroup);
  }
  return retval;
}


C_FUNC osMessageQueueId_t osBiGroupGetMailQ(osBiGroupId biGroup) {
  return biGroup->m_messageQ;
}


void osBiStateChangeEventsFree(osBiGroupId biGroup, osBiStateChangeEvents *stateChangeEvents){
  //osMailFree(biGroup->m_mailQ, stateChangeEvents);
}


/**
 * \ingroup _cmsis_os_ext_bi
 */
//STATIC bapi_E_BiChannel _osBiGetStateChange(unsigned int index, unsigned int bitNo, const osBiStateChangeEvents event, bool* state) {
STATIC bapi_E_BiChannel _osBiGetStateChange(unsigned int index, unsigned int bitNo, const osBiStateChangeEvents* stateChangeEvents, bool* state) {
  //if(event->status == osEventMail) {
    //osBiStateChangeEvents *stateChangeEvents = S_CAST(osBiStateChangeEvents *, event->value.p);

#ifdef BIGROUP_DEBUG
    ASSERT(stateChangeEvents->m_mailTag == BIGROUP_MAILTAG);
#endif

    for(; index < BITFIELD_ARRAY_SIZE; index++) {
      for(; bitNo < BITS_PER_BYTE * sizeof(bitfield_base_t); bitNo++) {
        if( stateChangeEvents->m_biStateChangeBitfield[index] & (1 << bitNo)) {
          int biChannel = sizeof(bitfield_base_t) * index + bitNo;
          if(biChannel < bapi_bi_E_Ch_Count) {
            *state = stateChangeEvents->m_biStateBitfield[index] & (1 << bitNo);
          } else {
            biChannel = bapi_bi_E_Invalid;
          }
          return S_CAST(bapi_E_BiChannel, biChannel);
        }
      }
    }
  //}
  return bapi_bi_E_Invalid;
}

//bapi_E_BiChannel osBiGetFirstStateChange(const osBiStateChangeEvents event, bool* state) {
bapi_E_BiChannel osBiGetFirstStateChange(const osBiStateChangeEvents* stateChangeEvents, bool* state) {
  return _osBiGetStateChange(0, 0, stateChangeEvents, state);
}

//bapi_E_BiChannel osBiGetNextStateChange(const osBiStateChangeEvents event, bool* state, bapi_E_BiChannel predecessor) {
bapi_E_BiChannel osBiGetNextStateChange(const osBiStateChangeEvents* stateChangeEvents, bool* state, bapi_E_BiChannel predecessor) {
  const int start = S_CAST(int, predecessor) + 1;
  const unsigned int index = start / sizeof(bitfield_base_t);
  const unsigned int bitNo = start % sizeof(bitfield_base_t);
  return _osBiGetStateChange(index, bitNo, stateChangeEvents, state);
}

//h242608
uint64_t osBiGetPulseCounter(const osBiStateChangeEvents* stateChangeEvents, bapi_E_BiChannel channel)
{
    return stateChangeEvents->pulse_counter[channel];
}

void osResetBiPulseCounter(osBiGroupId biGroup,bapi_E_BiChannel channel)
{
	biGroup->m_groupMembers.resetBiPulseCounter(channel);
}

void osSetBiPulseCounter(osBiGroupId biGroup,bapi_E_BiChannel channel, uint64_t count)
{
	biGroup->m_groupMembers.setBiPulseCounter(channel,count);
}


