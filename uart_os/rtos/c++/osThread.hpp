/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

#ifndef osThread_HPP_INCLUDED
#define osThread_HPP_INCLUDED

#if defined(__cplusplus)

#include "baseplate.h"
#include "rtos/cmsis-rtos/cmsis_os_redirect.h"

/**
 * \file
 * \brief This file implements the class os::Thread.
 */

/**
 * \ingroup cmsis_os_cpp
 * \brief namespace for CMSIS - RTOS C++ wrappers
 */
namespace os {

/**
 * \ingroup cmsis_os_cpp
 * \brief
 * This class implements a CMSIS thread that with basic thread
 *   API functions as member functions. It cannot be instantiated
 *   since it is supposed to operate only as the base class
 *   for the template class os::Thread. If there is a need
 *   to pass a generic pointer to an instance of a class os::Thread,
 *   a pointer to this base class can be passed instead. The
 *   advantage is, that a ThreadBase pointer can be used
 *   for any thread, because it does not depend on a template
 *   parameter.
 */
class ThreadBase {

protected:
#if TARGET_RTOS != RTOS_NoRTOS
  osThreadId_t m_threadId;

  ThreadBase(void) : m_threadId(0) {
    return;
  }

  virtual ~ThreadBase() {
    Terminate();
  }
#endif

#if TARGET_RTOS != RTOS_NoRTOS

  osThreadId_t _Create(
    osThreadFunc_t _pthread,     /**< start address of thread function */
    osPriority_t   tpriority,    /**< initial thread priority */
    uint32_t       stacksize,    /**< stack size requirements in bytes; 0 is default stack size */
    const char*    name,         /**< A descriptive name for the task. */
    uint32_t       instances     /**< maximum number of instances of that thread function */
  ) {
    ASSERT(m_threadId==0);

    //os_thread_def threadDef;
    //threadDef.tpriority = tpriority;
    //threadDef.stacksize = stacksize;
    //threadDef.name = name;
    //threadDef.instances = instances;
    //threadDef.pthread = _pthread;
    //m_threadId = osThreadCreate(&threadDef, this);

    ASSERT_DEBUG(tpriority <= configMAX_PRIORITIES);

    osThreadAttr_t threadAtt = {name, 0, NULL, 0, NULL, stacksize, tpriority, 0, 0};
    m_threadId = osThreadNew(_pthread, this, &threadAtt);

    return m_threadId;
  }

#endif

public:

#if TARGET_RTOS != RTOS_NoRTOS
  virtual osThreadId_t Create(
    osPriority_t tpriority,    /**< initial thread priority */
    uint32_t     stacksize,    /**< stack size requirements in bytes; 0 is default stack size */
    const char*  name,         /**< A descriptive name for the task. */
    uint32_t     instances = 1 /**< maximum number of instances of that thread function */
  )=0;
#endif

#if TARGET_RTOS != RTOS_NoRTOS
  inline osThreadId_t GetId(void)const {return m_threadId;}
#else
  inline osThreadId_t GetId(void)const {return osThreadGetId();}
#endif

#if TARGET_RTOS != RTOS_NoRTOS
  static inline osStatus_t Terminate(osThreadId_t thread_id){

#if INCLUDE_vTaskDelete
    return osThreadTerminate(thread_id);
#else
    return osError;
#endif

  }

  inline osStatus_t Terminate(){
    return Terminate(m_threadId);
  }
#endif

  static inline osStatus_t Yield(void){
    return osThreadYield();
  }

  static inline osStatus_t SetPriority (osThreadId_t thread_id, osPriority_t priority){
    return osThreadSetPriority(thread_id, priority);
  }

  inline osStatus_t SetPriority (osPriority_t priority){
    return osThreadSetPriority(GetId(), priority);
  }

  inline osPriority_t GetPriority (osThreadId_t thread_id){
    return osThreadGetPriority(thread_id);
  }

  inline osPriority_t GetPriority ()const{
    return osThreadGetPriority(GetId());
  }

  inline int32_t SignalSet(int32_t signals) {
    //return osSignalSet(GetId(), signals);
    return osThreadFlagsSet(GetId(), signals);
  }

  inline osThreadState_t State() const {
#if TARGET_RTOS != RTOS_NoRTOS
    //return osGetThreadState_suppl(m_threadId);
    return osThreadGetState(m_threadId);
#else
    return osGetThreadState_suppl(0);
#endif
  }

  inline osStatus_t Suspend() {
#if TARGET_RTOS != RTOS_NoRTOS
    //return osThreadSuspend_suppl(m_threadId);
    return osThreadSuspend(m_threadId);
#else
    return osThreadSuspend_suppl(0);
#endif
  }

  inline osStatus_t Resume() {
#if TARGET_RTOS != RTOS_NoRTOS
    //return osThreadResume_suppl(m_threadId);
    return osThreadResume(m_threadId);
#else
    return osThreadResume_suppl(0);
#endif
  }

  inline const char* Name() {
#if TARGET_RTOS != RTOS_NoRTOS
//    return osThreadGetName_suppl(m_threadId);
    return osThreadGetName(m_threadId);
#else
    return "?";
#endif
  }

};


/**
 * \ingroup cmsis_os_cpp
 * \brief 
 * This class implements a CMSIS thread that will execute a
 * thread function that is a member of a derived class.
 *
 *
 * Usage:
 * \code
 *  struct MyThread : public class Thread<MyThread> {
 *    void pthread() {
 *      ...
 *    }
 *  };
 * \endcode
 *
 * \note The wrapper does not generate code overhead, because it uses inline
 * functions for wrapping.
 */
template< typename derived > class Thread : public ThreadBase {

protected:
  static void _pthread(void * _this) {
    static_cast<derived*>(const_cast<void*>(_this))->pthread();
    osThreadExit();
  }

public:
  inline Thread(void) {
    return;
  }

#if TARGET_RTOS != RTOS_NoRTOS

  virtual ~Thread() {
  }

  virtual osThreadId_t Create(
    osPriority_t tpriority,    /**< initial thread priority */
    uint32_t     stacksize,    /**< stack size requirements in bytes; 0 is default stack size */
    const char*  name,         /**< A descriptive name for the task. */
    uint32_t     instances = 1 /**< maximum number of instances of that thread function */
  ) {
    return _Create(_pthread, tpriority, stacksize, name, instances);
  }

protected:
  /* Wait for one or more Signal Flags to become signaled for \b this RUNNING thread. */
  //static inline osEvent SignalWait(uint32_t millisec = osWaitForever, int32_t signals=0) {
  static inline osStatus_t SignalWait(uint32_t millisec = osWaitForever, int32_t signals=0) {
    //return osSignalWait(signals, millisec);

#define SignalMask ((1U<<osFeature_Signals)-1U)
    osStatus_t retStatus;
    uint32_t flags;
    if (signals != 0) {
      flags = osThreadFlagsWait((uint32_t)signals, osFlagsWaitAll, millisec);
    } else {
      flags = osThreadFlagsWait(SignalMask,        osFlagsWaitAny, millisec);
    }
    if ((flags > 0U) && (flags < 0x80000000U)) {
      retStatus = osOK;
    } else {
      retStatus = (osStatus_t)flags;
    }
    return retStatus;
  }
#endif
};

} /* namespace os */

#endif // #if defined(__cplusplus)

#endif /* #ifdef osThread_HPP_INCLUDED */
