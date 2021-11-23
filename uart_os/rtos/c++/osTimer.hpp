/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */



#ifndef osTimer_HPP_INCLUDED
#define osTimer_HPP_INCLUDED

#if defined(__cplusplus)


#include "baseplate.h"
#include "rtos/cmsis-rtos/cmsis_os_redirect.h"

/**
 * \file
 * \brief This file implements the class os::Timer.
 */

namespace os {

class TimerBase {

protected:
  osTimerId_t m_timerId;

  TimerBase(void) : m_timerId(0) {
    return;
  }

  inline ~TimerBase (){
    Delete();
  }

  inline osTimerId_t _Create(
      osTimerFunc_t _onTimeout,
      osTimerType_t type,
    const char* name
  ) {
    ASSERT(m_timerId==0);

    //os_timer_def timer_def;
    //timer_def.ptimer = _onTimeout;
    //timer_def.name = name;
    //m_timerId = osTimerCreate(&timer_def, type, this);

    osTimerAttr_t attr  = {name, 0, NULL, 0};
    m_timerId = osTimerNew(_onTimeout, type, this, &attr);
    return m_timerId;
  }

public:
  virtual osTimerId_t Create(osTimerType_t type, const char* name = 0) = 0;

  inline osTimerId_t GetId(void)const {return m_timerId;}

  inline osStatus_t Start(uint32_t millisec) {
    return osTimerStart(m_timerId, millisec);
  }

  inline osStatus_t Stop(){
    return osTimerStop(m_timerId);
  }

  inline osStatus_t Delete(){
    osStatus_t retval = osTimerDelete(m_timerId);
    if(retval == osOK) {
      m_timerId = 0;
    }
    return retval;
  }

  inline bool isActive()const{
    //return osTimerIsActive_suppl(m_timerId);
    return osTimerIsRunning(m_timerId);
  }
};

/**
 * \ingroup cmsis_os_cpp
 * \brief* This class implements a CMSIS timer that will execute a
 * timeout function that is a member of a derived class.
 *
 * Static inheritance is used to avoid the overhead of a virtual function table.
 *
 * Usage:
 * \code
 *  struct MyTimer : public class Timer<MyTimer> {
 *    void expired() {
 *      ...
 *    }
 *  };
 * \endcode
 * \note The wrapper does not generate code overhead, because it uses inline
 * functions for wrapping.
 */
template< typename derived > class Timer : public TimerBase {

protected:
  osTimerId_t m_timerId;

  static void _onTimeout(void* _this) {
    static_cast<derived*>(const_cast<void*>(_this))->onTimeout();
  }

public:

  virtual ~Timer (){
  }

  virtual osTimerId_t Create(osTimerType_t type, const char* name = 0
  ) {
    return _Create(_onTimeout, type, name);
  }
};

} /* namespace os */

#endif // #if defined(__cplusplus)


#endif /* #ifndef osTimer_HPP_INCLUDED */
