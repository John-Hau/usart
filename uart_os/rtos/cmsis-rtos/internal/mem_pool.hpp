/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */


#include "build-config.h" /* This must be the first include */
#ifndef mem_pool_H_
#define mem_pool_H_


#if TARGET_RTOS != RTOS_ThreadX /* ThreadX uses its own pool implementation. */

#include "baseplate.h"
#ifdef __IAR_SYSTEMS_LIB__
  #include <stdlib.h>
#else
  #include <malloc.h>
#endif
#include <string.h>

#include <new>
#include <limits>

#include "boards/board-api/bapi_irq.h"
#include "boards/board-api/bapi_atomic.h"


#include "rtos/cmsis-rtos/internal/mem_pool_destructor.hpp"

/**
 * \file
 * \brief
 * Implements the cmsis RTOS extension internal structure _os::MemoryPool.
 */

/**
 * \ingroup _cmsis_os
 * \brief The _os namespace is used as internal namespace for providing the CMSIS RTOS API.
 */
namespace _os {

/**
 * \ingroup _cmsis_os
 * \brief A template class that implements Memory Pools as per CMSIS RTOS.
 * As an extension to the CMSIS RTOS API, a graceful destruction feature is supported either.
 *
 * The MemoryPool provides a fixed number of fixed size memory blocks to be allocated
 * and freed. The number of blocks and the block size is determined at the MemoryPool
 * creation time. Memory blocks can be allocated and freed in the thread context as well as
 * in the ISR context.
 *
 * \note All memory that is required for a MemoryPool is allocated in one shot. That means that
 * the memory blocks and some administration data (pool occupancy flags) are always placed
 * immediately behind the MemoryPool structure. This ensures a better heap utilization and
 * avoids multiple memory allocation calls which would need to be checked whether they were
 * successful, which in turn would require freeing up already allocated memory in case it
 * failed.
 *
 */
template<typename WAIT_TICK_PROVIDER, typename INDEX_TYPE = uint16_t> struct MemoryPool {

  template<typename> friend struct MemoryPoolDestructor;
  typedef INDEX_TYPE index_type;
  typedef struct MemoryPoolDestructor<MemoryPool> destructor;

private:

  static inline MsecType waitSingleTick(MsecType msec) {
    MsecType result = WAIT_TICK_PROVIDER::waitSingleTick(msec);
    if(msec != osWaitForever) {
      return result;
    }
    return osWaitForever;
  }

  enum {
    /* For better readability we use this symbol instead of sizeof(int) */
     POOL_ALIGNMENT = sizeof(int)
    /* For better readability we use this symbol instead of a hard coded 8 */
    ,BITS_PER_BYTE = 8
  };

  /**
   * Only nonzero while a thread is destroying the memory pool. When
   * this member is nonzero, freeing blocks is still possible, but
   * block allocation is not.
   */
  osThreadId m_destroyingThread;

  /** The size of one block in the memory pool. */
  size_t m_blockSize;

  /** Number of blocks in the memory pool. */
  index_type m_maxBlocks;

  /**
   * Data type for flags that give information which blocks are currently
   * occupied (1), respectively available (0).
   * */
  typedef uint32_t pool_occupancy_t;

  /**
   *  Find any bit position of a zero set bit in an pool_occupancy_t variable.
   */
  inline index_type find_unused(pool_occupancy_t poolOccupancyItem) {
    index_type retval = 0;

    /* Check if there is at least 1 zero bit */
    ASSERT(poolOccupancyItem != std::numeric_limits<pool_occupancy_t>::max());

    /* Find the lowest zero bit.  */
#ifdef LINEAR_ZERO_BIT_SEARCH
    pool_occupancy_t mask = 1ul;
    while(poolOccupancyItem & (mask << retval)) { /* This loop is executed32 time in worst case */
      ++retval;
    }
#else
    register uint16_t bitShift = BITS_PER_BYTE * sizeof(pool_occupancy_t) / 2;
    register pool_occupancy_t testMask = ((1ul << bitShift) - 1);
    while ( bitShift ) { /* This loop is always executed 4 times */
      register uint32_t shiftedPoolOccupancyItem = (poolOccupancyItem >> retval);
      if ((shiftedPoolOccupancyItem & testMask) == testMask) {
        retval = retval + bitShift;
      }
      bitShift = bitShift / 2;
      testMask = ((1ul << bitShift) - 1);
    }
#endif

    ASSERT((poolOccupancyItem & (1ul << retval)) == 0);
    return retval;
  }
  
  /**
   * Get pointer to the memory location where the poolOccupancy flags area starts.
   * */
  inline pool_occupancy_t* poolOccupancy() {
    return R_CAST(pool_occupancy_t* ,R_CAST(uint8_t*, this) + sizeof(MemoryPool));
  }

  /**
   * Get const pointer to the memory location where the poolOccupancy flags area starts.
   * */
  inline const pool_occupancy_t* poolOccupancy()const {
    return R_CAST(const pool_occupancy_t* ,R_CAST(const uint8_t*, this) + sizeof(MemoryPool));
  }

  /**
   * Get pointer to the memory location where the poolBlock area starts.
   * */
  inline uint8_t* poolBlocks() {
    return R_CAST(uint8_t*, this) + sizeof(MemoryPool) + poolOccupancySize(m_maxBlocks);
  }

  /**
   * Get const pointer to the memory location where the poolBlock area starts.
   * */
  inline const uint8_t* poolBlocks()const {
    return R_CAST(const uint8_t*, this) + sizeof(MemoryPool) + poolOccupancySize(m_maxBlocks);
  }

  /** Calculate the number of pool_occupancy_t values required to store the occupied flags. */
  static inline index_type poolOccupancyCount(index_type maxBlocks) {
    return (maxBlocks + (BITS_PER_BYTE * sizeof(pool_occupancy_t)) - 1)
      / (BITS_PER_BYTE * sizeof(pool_occupancy_t));
  }


  /** Calculate the number of bytes required to store the occupied flags. */
  static inline size_t poolOccupancySize(index_type maxBlocks) {
    return poolOccupancyCount(maxBlocks) * sizeof(pool_occupancy_t);
  }

  /** Calculate the number of bytes required to store all the memory blocks. */
  static inline size_t poolBlocksSize(index_type maxBlocks, uint32_t blockSize) {
    return static_cast<size_t>(maxBlocks * blockSize);
  }

  /** Retrieve if is there is no memory block in use. */
  bool empty()const {
    index_type i = poolOccupancyCount(m_maxBlocks);
    while(i) {
      --i;
      if(poolOccupancy()[i]) {
        return false;
      }
    }
    return true;
  }

  /**
   * \brief
   * The constructor is private, because there must be more memory allocated to store the
   * poolOccupancy flags and the poolBlocks behind this structure. Hence the static
   * create(index_type maxItems, uint32_t itemSize) function must be used instead of
   * a direct construction via news. */
  inline MemoryPool(index_type maxItems, uint32_t itemSize)
    : m_destroyingThread(0)
    , m_blockSize((((itemSize + POOL_ALIGNMENT - 1) / POOL_ALIGNMENT) * POOL_ALIGNMENT))
    , m_maxBlocks(maxItems) {
  }

  /**
   * No destructor call allowed. Call openDestruction() and destroy(MsecType millisec, bool bClose) instead.
   */
  ~MemoryPool();

public:
  static MemoryPool* create(index_type maxBlocks, uint32_t blockSize) {
    /* Allocate memory that can take the poolOccupancy flags and the poolBlocks
     * behind this MemoryPool structure. */
    size_t mallocSize = sizeof(MemoryPool) + poolOccupancySize(maxBlocks) + poolBlocksSize(maxBlocks, blockSize);
    void* place = ::malloc(mallocSize);

    if(place) {
      memset(place, 0, mallocSize);

      /* Use placement new operator to place the Memory pool at the allocated memory. */
      return new(place) MemoryPool(maxBlocks, blockSize);
    }

    return 0;
  }

//  /**
//   * Must be called before calling the destroy method, which will exclusively reserve a following
//   * destroy(MsecType millisec, bool bClose) call for the currently running thread. It will also disable any
//   * block allocation from the pool.
//   * \return true, if destroy(MsecType millisec) can be called. false, if another
//   * thread is already performing the destruction.
//   */
//  bool openDestruction() {
//    bool retval = false;
//    bapi_irq_enterCritical();
//    if(this) {
//      if(!m_destroyingThread) {
//        m_destroyingThread = osThreadGetId();
//        retval = true;
//      }
//    }
//    bapi_irq_exitCritical();
//    return retval;
//  }

//  bool assertDestroyingThread()const {
//    bool retval = false;
//    bapi_irq_enterCritical();
//    retval = this && (m_destroyingThread == osThreadGetId());
//    bapi_irq_exitCritical();
//    return retval;
//  }
//
//  bool isDestructing()const {
//    bool retval = false;
//    bapi_irq_enterCritical();
//    if(!this || m_destroyingThread) {
//      retval = true;
//    }
//    bapi_irq_exitCritical();
//    return retval;
//  }

//  /**
//   * Must be called after the destroy method with the return value of the destroy method. In case
//   * the destroy method failed this method will withdraw the reservation of the
//   * destroy(MsecType millisec) call for a particular thread and will re-allow block allocation.
//   */
//  void closeDestruction(MemoryPool* *const enclosure, bool success) {
//    ASSERT(m_destroyingThread == osThreadGetId());
//    if(success) {
//      if(enclosure) {
//        *enclosure = 0;
//      }
//      free(this);
//    } else {
//      m_destroyingThread = 0;
//    }
//  }

  /**
   * Try Destroying this memory pool.
   * \return true if the memory pool was successfully destroyed. Otherwise false.
   */
  bool destroy(MsecType millisec, bool bClose) {

    ASSERT(m_destroyingThread == osThreadGetId());
    ASSERT(m_maxBlocks && m_blockSize);

    /* Lower thread priority to give other threads the chance to
     * free up memory blocks. */
    osPriority originalThreadPrio = osThreadGetPriority(m_destroyingThread);
    osThreadSetPriority(m_destroyingThread, osPriorityIdle);

    /* We wait until the pool can be destroyed. */
    while ( !empty() && millisec ) {
      millisec = waitSingleTick(millisec);
    }

    /* Set back to original priority. */
    osThreadSetPriority(m_destroyingThread, originalThreadPrio);

    bool success = empty();

    /* See if we could empty the pool within the timeout period. */
    /* yes, so free up our memory and return null. */
    if(bClose) {
      destructor::closeDestruction(this, 0, success);
    }

    /* Memory could not be destroyed, so we return false. */
    return success;
  }
  
  inline size_t blockSize()const {
    size_t retval = 0;
    bapi_irq_enterCritical();
    if(this) {
      retval = m_blockSize;
    }
    bapi_irq_exitCritical();
    return retval;
  }

  inline size_t maxBlocks()const {
    return m_maxBlocks;
  }

  inline index_type toIndex(const void* const address) {
    return (static_cast<const uint8_t* const >(address) - at(0)) / m_blockSize;
  }

  inline uint8_t* at(index_type index) {
    return &(poolBlocks()[index * m_blockSize]);
  }

  inline void* alloc(MsecType msecBlockTime, index_type size = 0) {
    void* retval = 0;

    if (!destructor::isDestructing(this)) {
      do {
        /* Ensure atomic operation. */
        bapi_irq_enterCritical();
        if (size < m_maxBlocks) {
          /* There is a free entry, so let's find it. */
          index_type poolOccupancyIndex = 0;
          for ( ; !retval && (poolOccupancyIndex < poolOccupancyCount(m_maxBlocks)); poolOccupancyIndex++ ) {

            if (poolOccupancy()[poolOccupancyIndex] != std::numeric_limits<pool_occupancy_t>::max()) {

              /** We found an free entry */
              index_type bitNo = find_unused(poolOccupancy()[poolOccupancyIndex]);
              index_type poolItemIndex = (BITS_PER_BYTE * sizeof(pool_occupancy_t)) * poolOccupancyIndex + bitNo;

              if(poolItemIndex < m_maxBlocks) {
                poolOccupancy()[poolOccupancyIndex] |= 1ul << bitNo;
                retval = &(poolBlocks()[poolItemIndex * m_blockSize]);
              } else {
                break;
              }
            }
          }
        }
        bapi_irq_exitCritical();
        if(retval) {
          break;
        }
        msecBlockTime = WAIT_TICK_PROVIDER::waitSingleTick(msecBlockTime);
      } while ( msecBlockTime );
    }
    return retval;
  }

  inline void* calloc(MsecType msecBlockTime, index_type size = 0) {
    void* retval = alloc(msecBlockTime, size);
    if (retval) {
      memset(retval, 0, m_blockSize);
    }
    return retval;
  }

  osStatus freeBlock(const void* pBlock) {
    if(pBlock) {
      /* Assert that the item is really from this pool. */
      ASSERT((pBlock >= &(poolBlocks()[0])) && (pBlock < &(poolBlocks()[m_maxBlocks * m_blockSize])));

      index_type poolItemIndex = (static_cast<const uint8_t*>(pBlock) - &(poolBlocks()[0])) / m_blockSize;
      index_type poolOccupancyIndex = poolItemIndex / (BITS_PER_BYTE * sizeof(pool_occupancy_t));
      index_type bitNo = poolItemIndex % (BITS_PER_BYTE * sizeof(pool_occupancy_t));

      atomic_bitwiseAND(&(poolOccupancy()[poolOccupancyIndex * m_blockSize]), ~(1ul << bitNo));
    }
    return osOK;
  }

};

} /* namespace _os */

#endif /* #if TARGET_RTOS != RTOS_ThreadX */
#endif /* #ifndef mem_pool_H_ */

