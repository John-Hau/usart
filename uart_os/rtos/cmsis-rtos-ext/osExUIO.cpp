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

#include "osExUIO.h"
#include "rtos/c++/osMailQueue.hpp"
#include "cmsis_os2.h"

#if defined (FS_IMXRTEVAL)
#include "bapi_io_smbio_FS_IMXRTEVAL.h"
#elif defined (FS_BEATS_IO)
#include "bapi_io_smbio_FS_BEATS_IO.h"
#elif defined (FS_IPVAV)
#include "bapi_uio_FS_IPVAV.h"
#elif defined (FS_SNAP_ON_IO)
#include "bapi_uio_FS_SNAP_ON_IO.h"
#endif

osMutexId_t uioCongigMutex;
osMutexId_t uioMutex;


#define UIO_AI_SAMPLE_BATCH_NUM  5 
bool notFirstReadUIOAI[TOTALUIOCHANEL] = {0};
typedef struct osUioAiSampleBatch 
{  
    uint8_t     m_sampleIndex;  
    uint8_t    m_sampleCount;  
    uint16_t  m_sampleBatch[UIO_AI_SAMPLE_BATCH_NUM]; 
} osUioAiSampleBatch_t; 

osUioAiSampleBatch_t UioAiSampleBatch[UIO_CHANNEL_TOTAL];

uint16_t osExUioAiAverage(uint8_t uioPinIndex, uint16_t val)  
{   
    uint32_t sum = 0;
    UioAiSampleBatch[uioPinIndex].m_sampleBatch[UioAiSampleBatch[uioPinIndex].m_sampleIndex] = val; 
    ++UioAiSampleBatch[uioPinIndex].m_sampleIndex;  
    ++UioAiSampleBatch[uioPinIndex].m_sampleCount; 

    if( UioAiSampleBatch[uioPinIndex].m_sampleIndex >= UIO_AI_SAMPLE_BATCH_NUM )  
    {      
    	UioAiSampleBatch[uioPinIndex].m_sampleIndex = 0;  
    }  
	
    if( UioAiSampleBatch[uioPinIndex].m_sampleCount > UIO_AI_SAMPLE_BATCH_NUM)  
    {      
    	UioAiSampleBatch[uioPinIndex].m_sampleCount = UIO_AI_SAMPLE_BATCH_NUM;  
    }  
	
    for(int i = 0; i<UioAiSampleBatch[uioPinIndex].m_sampleCount; i++)  
    {      
    	sum += UioAiSampleBatch[uioPinIndex].m_sampleBatch[i];  
    }

    return sum/UioAiSampleBatch[uioPinIndex].m_sampleCount; 
}


void osExUioMutexCreat(void)
{

	osMutexAttr_t mutexDef2 = {"ExUioValueMutex", osMutexRecursive, NULL, 0};
	uioMutex = osMutexNew(&mutexDef2);

}


bool osExUioCongigAiChanel(uint8_t uioPinIndex,IO_PIN_TYPE_t pintype)
{
    bapi_io_uio_configure(pintype,uioPinIndex);
	UioAiSampleBatch[uioPinIndex].m_sampleCount = 0;
	UioAiSampleBatch[uioPinIndex].m_sampleIndex = 0;
	memset(&UioAiSampleBatch[uioPinIndex].m_sampleBatch[0],0,UIO_AI_SAMPLE_BATCH_NUM);
	notFirstReadUIOAI[uioPinIndex] = false;
}

bool osExUioCongigBiChanel(uint8_t uioPinIndex,IO_PIN_TYPE_t pintype)
{
	return (bapi_io_uio_configure(pintype,uioPinIndex));
}

bool osExUioCongigAoChanel(uint8_t uioPinIndex,IO_PIN_TYPE_t pintype)
{
	bapi_io_uio_configure(pintype,uioPinIndex);
}

bool osExUioConfigChanel(uint8_t uioPinIndex,IO_PIN_TYPE_t pintype)
{
	bapi_io_uio_configure(pintype,uioPinIndex);
}


uint16_t osExUioGetAIValue(uint8_t uioPinIndex)
{
    UioValue retvalue;
	osMutexAcquire(uioMutex, osWaitForever);

    retvalue = bapi_io_uio_getValue(uioPinIndex);
	
	osMutexRelease(uioMutex);
//	if(!notFirstReadUIOAI[uioPinIndex])
//	{
		return retvalue.aiRawValue;
//	}
//	else
//	{
//	    return osExUioAiAverage(uioPinIndex, retvalue.aiRawValue);
//	}
}

bool osExUioGetBIValue(uint8_t uioPinIndex)
{
    UioValue retvalue;
	osMutexAcquire(uioMutex, osWaitForever);

    retvalue = bapi_io_uio_getValue(uioPinIndex);
	
	osMutexRelease(uioMutex);
	
	return retvalue.biState;
}

/*
 * This function will read status of all 4 channels for selected UIO chip on selected SPI interface
 * */
uint8_t osExUioGet4BIValueinOneUIO(spi_bus_idx_t uio_SPIbusIndex,spi_dev_idx_t uio_deviceIndex)
{
    uint8_t retvalue;
	osMutexAcquire(uioMutex, osWaitForever);

//    retvalue = bapi_io_uio_getBIValues(SPI_BUS_UIO_1,SPI_DEV_UIO_1);
	retvalue = bapi_io_uio_getBIValues(uio_SPIbusIndex,uio_deviceIndex);
	osMutexRelease(uioMutex);
	
	return retvalue;
}



bool osExUioSetAOValue(uint8_t uioPinIndex,uint32_t value)
{
    bool retvalue;
	osMutexAcquire(uioMutex, osWaitForever);

    retvalue = bapi_io_uio_setValue(uioPinIndex,value);
	
	osMutexRelease(uioMutex);
	
	return retvalue;
}


uint8_t osExUioGetAlertStatus(void)
{
    uint8_t retvalue;
	uint8_t liveStatus[4] = {0};
	osMutexAcquire(uioMutex, osWaitForever);

    bapi_uio_read_LIVE_Status(SPI_BUS_UIO_1, SPI_DEV_UIO_1,liveStatus);
	
	osMutexRelease(uioMutex);
	retvalue = liveStatus[2]&0x0f;
	return retvalue;
}



