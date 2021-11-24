/*
 * timeMeasurement.h
 *
 *  Created on: 18.06.2016
 *      Author: Wolfgang
 */

#ifndef UTIL_TIME_MEASUREMENT_HPP_
#define UTIL_TIME_MEASUREMENT_HPP_

#include "boards/board-api/bapi_hwtimer.h"

template<unsigned MAX_COUNT, typename ITEM_TYPE> class TimeStampedSeries {

public:
  enum{ TIMER_RUNTIME = 10000000 /* 10 seconds */ };
  enum{ SERIES_MAX_LEN = MAX_COUNT};

  typedef ITEM_TYPE item_t;

  struct TimeStampedItem {
    uint32_t timeStamp;
    item_t   item;
  };

private:

  typedef bapi_HwtimerHandle  timerHandle_t;

  timerHandle_t timerHandle;

  uint32_t overflowCounter;
  uint32_t startTime;
  unsigned seriesLen;


  struct TimeStampedItem series[MAX_COUNT];

  inline void clear() {
    seriesLen = 0;
    overflowCounter = 0;
  }

  static void callback(void* userData) {
    TimeStampedSeries* _this = S_CAST(TimeStampedSeries*, userData);
    _this->overflowCounter++;
  }

  void ensureMeasurementTimer() {
    if (!timerHandle) {
      timerHandle = bapi_hwt_allocateHardwareTimer();
      if (timerHandle) {
        bapi_hwt_configureTimer(timerHandle,
          bapi_Hwt_E_Periodic, TIMER_RUNTIME
          );
        bapi_hwt_installTimeoutCallback(timerHandle, callback, S_CAST(void*, this));
      }
    }
  }

public:
  TimeStampedSeries()
    : overflowCounter(0), timerHandle(0), seriesLen(0) {
  }

  ~TimeStampedSeries() {
    stopMeasurement(true);
  }

  /**
   * @return The length of the series.
   */
  inline unsigned getSeries(struct TimeStampedItem buffer[MAX_COUNT])const {
    bapi_irq_enterCritical();
    unsigned retval = seriesLen;
    memcpy(buffer, series, seriesLen * sizeof(series[0]));
    bapi_irq_exitCritical();
    return retval;
  }

  void stopMeasurement(bool bReleaseHwTimer = false) {
    bapi_irq_enterCritical();
    if (timerHandle) {
      bapi_hwt_stopTimer(timerHandle);
      if(bReleaseHwTimer) {
        timerHandle_t tmpTimerHandle = timerHandle;
        timerHandle = 0;
        clear();
        bapi_irq_exitCritical();
        bapi_hwt_releaseTimer(tmpTimerHandle);
        return;
      }
    }
    bapi_irq_exitCritical();
  }


  bool startMeasurement(const item_t& firstItem) {
    bool retval = false;

    ensureMeasurementTimer();

    bapi_irq_enterCritical();

    if (timerHandle) {
      bapi_hwt_stopTimer(timerHandle);
      clear();

      retval = bapi_hwt_startTimer(timerHandle);
      if(retval) {
        series[seriesLen].item = firstItem;
        series[seriesLen].timeStamp = bapi_hwt_readTimerUs(timerHandle);

        /* Select next series entry. */
        seriesLen++;
      }
    }

    bapi_irq_exitCritical();

    return retval;
  }
  
  bool takeTimeStamp(const item_t& nextItem) {

    bool retval = false;


    bapi_irq_enterCritical();

    if (timerHandle) {
      /* Measure asap. */
      uint32_t timeStamp = bapi_hwt_readTimerUs(timerHandle);
      bapi_irq_exitCritical();

      /* Give interrupts a chance to step in here. */

      bapi_irq_enterCritical();
      if (seriesLen < ARRAY_SIZE(series)) {

        /* There is a free entry in the samples array. */
        series[seriesLen].timeStamp = timeStamp;
        series[seriesLen].item = nextItem;

        /* Select next series entry. */
        seriesLen++;

        /* Success ! */
        retval = true;
      }

    }

    bapi_irq_exitCritical();

    return retval;
  }
};

#endif /* UTIL_TIME_MEASUREMENT_HPP_ */
