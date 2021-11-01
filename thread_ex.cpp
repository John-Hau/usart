NORETURN void bapi_fatalError(char const* UNUSED(file), const unsigned int UNUSED(line))
{
	bapi_irq_enterCritical();

	ui_manager_force_set_mainled_red();
	PRINTF( "bapi_fatalError: %s, %u!!!\r\n", file, line );
	while(1);
}


 #define ASSERT(expr) if(!(expr)) { bapi_fatalError(__FILE__, __LINE__); }




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


class HeartBeatThread : public os::Thread<HeartBeatThread> {
public:
  enum {maxThreads = 13};

private:
  typedef os::Thread<HeartBeatThread> myBaseClass;

  static HeartBeatThread* m_registeredThreads[maxThreads];

  static void registerThread(HeartBeatThread* thread);

  bapi_SystemTick_t nextPulseRequired;

public:
  bool m_bThreadRunning;
  bool m_bStopThread;

protected:
  inline void heartBeat(bapi_SystemTick_t pulsePeriod) {
    atomic_Set(&nextPulseRequired, osKernelGetSysTimerCount()  + pulsePeriod);
  }

  inline void heartBeat(bapi_SystemTick_t pulsePeriod, bapi_SystemTick_t currentTick) {
    atomic_Set(&nextPulseRequired, currentTick + pulsePeriod);
  }

public:
  virtual void pthread() = 0;

  virtual osThreadId_t Create(
  osPriority_t   tpriority,    /**< initial thread priority */
    uint32_t     stacksize,    /**< stack size requirements in bytes; 0 is default stack size */
    const char*  name,         /**< A descriptive name for the task. */
    uint32_t     instances = 1 /**< maximum number of instances of that thread function */
  );

  HeartBeatThread(bapi_SystemTick_t pulsePeriod)
    : nextPulseRequired(osKernelGetSysTimerCount()  + pulsePeriod) {
    m_bThreadRunning = false;
    m_bStopThread = false;

  }

  virtual ~HeartBeatThread(){
    ASSERT(false); /* We don't support thread destruction. */
  }

  typedef signed_int<sizeof(bapi_SystemTick_t)>::type signedSysTick_t;

  signedSysTick_t nextHearBeatDistance(bapi_SystemTick_t currentTick)const {
    return S_CAST(signedSysTick_t, atomic_Get(&nextPulseRequired) - currentTick);
  }

  signedSysTick_t lastHearBeatDistance(bapi_SystemTick_t currentTick)const {
    return S_CAST(signedSysTick_t, currentTick - atomic_Get(&nextPulseRequired));
  }

  /**
   * Test if a registered thread is still alive
   */
  static inline bool isThreadAlive(size_t index, bapi_SystemTick_t currentTick) {
	  bool retVal = false;
	  if(m_registeredThreads[index]){
		  if(m_registeredThreads[index]->nextHearBeatDistance(currentTick) > 0){
			  retVal = true;
		  }
		  else{
			  iprintf("WDG FAILED FOR Index = %d %s\n", index, m_registeredThreads[index]->Name());
			  retVal =  false;
		  }
		  return retVal;
	  }
	  return (1);
  }

  /**
   * Test if a registered thread is still alive
   */
  static inline bool isThreadDead(size_t index, bapi_SystemTick_t currentTick) {
    bool retVal = false;
    if(m_registeredThreads[index]){
      if(m_registeredThreads[index]->lastHearBeatDistance(currentTick) >= DEAD_THREAD_TIMEOUT__IN_MSEC){
        iprintf("Dead Thread @ index = %d : %s\n", index, m_registeredThreads[index]->Name());
		iprintf("Dead Thread lastHearBeatDistance = %d, currentTick = %d \n", m_registeredThreads[index]->lastHearBeatDistance(currentTick),currentTick);
        retVal =  true;
      }
    }
    return retVal;
  }

  static HeartBeatThread* getAt(size_t index) {
    return m_registeredThreads[index];
  }

  static bool doAllThreadsAliveCheck();

  bool isThreadRunning(){return m_bThreadRunning;};
  void stopThread(){m_bStopThread = true;};
};
































struct UioBiThread : public HeartBeatThread {

  typedef HeartBeatThread myBaseClass;
  enum {
    initialHeartBeatPeriod = 30 * 10000
  };

  enum
  {
	  BiTaskPeriod = 2,
  };

  uint8_t periodBiCnt = 0;
  bool isFirstRun = true;
  

  UioBiThread(): myBaseClass(initialHeartBeatPeriod){}

  osThreadId_t Create(
    osPriority_t   tpriority,    /**< initial thread priority */
    uint32_t       stacksize,    /**< stack size requirements in bytes; 0 is default stack size */
    const char*    name,         /**< A descriptive name for the task. */
    uint32_t       instances = 1 /**< maximum number of instances of that thread function */
  ) 
  {
    return myBaseClass::Create(tpriority, stacksize, name, instances);
  }


	  void doWork()
	  {

	       int  bii=0;

	       bii +=1;

	  }

 
	  void pthread() 
	  {

	    uint8_t i=0;
	    while(1)
	    {
	      doWork();
	      osDelay(100);
	    }

	  }
};
