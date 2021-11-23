/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */



#ifndef osMutex_HPP_INCLUDED
#define osMutex_HPP_INCLUDED

#include "baseplate.h"
#include "rtos/cmsis-rtos/cmsis_os_redirect.h"

/**
 * \file
 * \brief A C++ wrapper os::Mutex for the osMutexId.
 */

namespace os {

class Mutex {

  osMutexId_t m_mutex;

public:
  Mutex() : m_mutex(0) {
  }

  ~Mutex() {
    osMutexDelete(m_mutex);
  }

  void create(const char* name = nullptr) volatile {
    if(!m_mutex) { /* Avoid second time creation. */
      //osMutexDef_t mutexDef;
      //mutexDef.name = name;
      //m_mutex = osMutexCreate(&mutexDef);
      osMutexAttr_t mutexDef = {name, osMutexRecursive, NULL, 0};
      m_mutex = osMutexNew(&mutexDef);
    }
  }

  inline osStatus_t wait(uint32_t millisec = osWaitForever) volatile {
    return osMutexAcquire(m_mutex, millisec);
  }

  inline osStatus_t release() volatile {
    return osMutexRelease(m_mutex);
  }

  inline bool isCreated()const volatile {
    return m_mutex != 0;
  }
};

} /* namespace os */

#endif /* #ifndef osMutex_HPP_INCLUDED */
