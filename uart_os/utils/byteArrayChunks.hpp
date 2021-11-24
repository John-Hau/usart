/*
 * byteArrayChunks.h
 *
 *  Created on: 20.06.2016
 *      Author: Wolfgang
 */

#ifndef UTILS_MEMORY_BLOCK_H_
#define UTILS_MEMORY_BLOCK_H_

#include <stdint.h>

namespace utils {


  /**
   * This is a utility class that provides  chunk information for an array of
   *   bytes. If the array need to be split into chunks. This is useful
   *   for programming flash device that can typically only programmed
   *   in chunk sizes (called pages.)
   *
   * n= number of chunks.
   *
   *    aligned start address
   *       |
   *       v
   *
   *       | chunkSize | chunkSize | chunkSize |     | chunkSize | chunkSize |
   *       v           v           v           v     v           v           v
   *       +-----------+-----------+-----------+.....+-----------+-----------+
   *       |    C0     |    C1     |    C2     |     |    Cn-2   |    Cn-1   |
   *       +-----------+-----------+-----------+.....+-----------+-----------+
   *              ^                                                    ^
   * Byte Array:  |-------------------- byteCount ---------------------|
   *             startAddress                                         endAddress
   *
   * Note: endAddress is the first address that does not belong to the
   *   Byte Array
   *
   *
 * __Code Example:__
 ~~~~~~~~{.cpp}


 bool flashProgramPage(const uint8_t* flashAddress, uint16_t byteCount);

 void flashProgramByteArray(uint32_t start, uint32_t count) {
                    :
   const ByteArrayChunks<uint32_t> chunks(start, count, getFlashPageSize();

   uint32_t chunkIndex = 0; uint32_t chunkCount = chunks.chunkCount();
   bool bOk = true;

   for(;bOk && (chunkIndex < chunkCount); chunkIndex++) {

     bOk = flashProgramPage(chunks.getChunkByteArrayAddress(), chunks.getChunkByteCount());
   }

   return bOk;
 }
 ~~~~~~~~
   *
   *
   */
	template<typename SIZE_T, typename ADDR_T, typename CHUNK_SIZE_ARRAY_T> class ByteArrayChunks {
	  typedef SIZE_T size_t;

	  typedef ADDR_T addr_t;

	  typedef CHUNK_SIZE_ARRAY_T chunk_size_array_t;

	  /* A non-zero chunk size array provides individual sizes for each chunk. */
	  const chunk_size_array_t* m_chunkSizeArray;

	  union {
	    size_t m_Index0_Offset; /* Used in case m_chunkSizeArray is set. */
      size_t m_chunkSize; /* Used in case we have uniform chunk sizes. */
	  };

    /* Start address is relative to the beginning of C0. */
	  addr_t m_startAddress;  /* Start address of the first chunk */
		size_t m_byteCount;     /* Number of bytes to cover. */
		
    inline size_t _chunkSize(uint32_t chunkIndex)const {
      if(m_chunkSizeArray) {
        return m_chunkSizeArray->chunkSizeAt(chunkIndex + m_Index0_Offset);
      }
      return m_chunkSize;
    }

    inline addr_t _alignedStartAddress(addr_t startAddress)const {

      if(m_chunkSizeArray) {
        /* We have chunks with different sizes. */

        addr_t retval = 0;

        /* Add all chunks that are before our first chunk. */
        uint32_t i = 0;
        while(i < m_Index0_Offset) {
          retval += m_chunkSizeArray->chunkSizeAt(i);
          i++;
        }

        return retval;
      }

      /* We have uniform chunks, so we just use the size of chunk 0 and
       * use it to align the start address. */
      const size_t chunk0 = _chunkSize(0);
      return (m_startAddress / chunk0) * chunk0;

    }

	public:
		/**
		 * Constructor that takes all necessary data to operate.
		 */
		inline ByteArrayChunks( SIZE_T chunkSize, addr_t startAddress, const size_t byteCount)
			: m_chunkSizeArray(nullptr), m_chunkSize(chunkSize), m_startAddress(R_CAST(addr_t, startAddress)), m_byteCount(byteCount) {
			}

    inline ByteArrayChunks(const chunk_size_array_t* chunkSizeArray, addr_t startAddress, const size_t byteCount)
      : m_chunkSizeArray(chunkSizeArray), m_chunkSize(0), m_startAddress(R_CAST(addr_t, startAddress)), m_byteCount(byteCount) {
      }

    inline ByteArrayChunks()
      : m_chunkSizeArray(nullptr), m_startAddress(0), m_byteCount(0), m_chunkSize(0) {
      }

    /**
     * Modify data to operate.
     */
    inline void setParam( SIZE_T chunkSize, addr_t startAddress, const size_t byteCount) {
      m_chunkSize = chunkSize; m_chunkSizeArray = nullptr; m_startAddress = startAddress; m_byteCount = byteCount;
    }

    /**
     * Modify data to operate.
     *
     *
     *  n = chunkCount
     *
     *
     *       | chunkSize | chunkSize | chunkSize |     | chunkSize | chunkSize |
     *       v           v           v           v     v           v           v
     *       +-----------+-----------+-----------+.....+-----------+-----------+
     *       |    C0     |    C1     |    C2     |     |    Cn-2   |    Cn-1   |
     *       +-----------+-----------+-----------+.....+-----------+-----------+
     *              ^                                                          ^
     * Byte Array:  |--------------- calculated byteCount ---------------------|
     *             startAddress
     *
     */
    inline void setParamByChunkCount( SIZE_T chunkSize, addr_t startAddress, const uint32_t chunkCount) {
       /* Calculate the byteCount from the chunkCount */

       m_chunkSize = chunkSize; m_chunkSizeArray = nullptr; m_startAddress = startAddress;

       const size_t cutOffBytes = (m_startAddress - _alignedStartAddress(startAddress));

       m_byteCount = (chunkCount * m_chunkSize) - cutOffBytes;
    }

    inline void setParamByChunkCount(const chunk_size_array_t* chunkSizeArray, addr_t startAddress, uint32_t chunkCount) {
       /* Calculate the byteCount from the chunkSizeArray */

       m_chunkSize = 0; m_chunkSizeArray = chunkSizeArray; m_startAddress = startAddress; m_byteCount = 0;

       uint32_t chunkIndex = 0;

       /* Find the first chunk that is above the start address. */
       uint32_t chunkAddr = 0;
       while(chunkAddr <= startAddress) {
         chunkAddr += m_chunkSizeArray->chunkSizeAt(chunkIndex);
         chunkIndex++;
       }

       /* Go back to last chunk that begins below or at the start address. */
       chunkIndex--; chunkAddr -= m_chunkSizeArray->chunkSizeAt(chunkIndex);

       /* Remember where to begin in the m_chunkSizeArray */
       m_Index0_Offset = chunkIndex;

       const size_t cutOffBytes = startAddress - chunkAddr;

       /* Add up all relevant chunk sizes. */
       while(chunkCount) {
         m_byteCount += m_chunkSizeArray->chunkSizeAt(chunkIndex);
         chunkIndex++;
         chunkCount--;
       }

       m_byteCount -= cutOffBytes;
    }

		/**
		 * Calculate the start address of chunk C0
		 */
		inline addr_t alignedStartAddress()const {
				return _alignedStartAddress(m_startAddress);
		}			

    /**
     * Calculate the first address behind chunk Cn-1
     */
		inline addr_t alignedEndAddress(uint32_t * chunkCount)const {
		  if(m_chunkSizeArray) {
        /* We have chunks with different sizes. */
		    addr_t alignedEndAddress  = alignedStartAddress();
		    uint32_t chunkIndex = m_Index0_Offset;

		    while(alignedEndAddress < m_startAddress + m_byteCount) {
		      alignedEndAddress += m_chunkSizeArray->chunkSizeAt(chunkIndex);
		      ++chunkIndex;
		    }

		    *chunkCount = chunkIndex - m_Index0_Offset;
        return alignedEndAddress;
		  } else {
		    /* We have uniform chunks */
		    const addr_t endAddress = m_startAddress + m_byteCount;
        const addr_t alignedEndAddress = (((endAddress-1)/m_chunkSize)+1) * m_chunkSize;

        *chunkCount = (alignedEndAddress - alignedStartAddress()) / m_chunkSize;
        return alignedEndAddress;
		  }
		}

    /**
     * Retrieve the required number of chunks to cover the Byte Array.
     */
		inline uint32_t chunkCount()const {
		  uint32_t retval = 0;
      alignedEndAddress(&retval);
		  return retval;
		}

    inline bool isEmpty()const {
      return m_byteCount == 0;
    }

    /**
     *  Calculate number of bytes from the startAddress to the end
     *  of the first Chunk. Special Case: The first chunk is the
     *  same than the last chunk.
     */
		inline size_t firstChunkByteCount()const {
			if(chunkCount() > 1) {
			  /* This is the standard case, where the last chunk is behind
			   * the first one.
			   */
			  const size_t offset = m_startAddress - alignedStartAddress();
				const size_t retval = _chunkSize(0) - offset;
        ASSERT(retval <= _chunkSize(0));
        return retval;
			}

			/* This handles the special case. */
			return m_byteCount;
		}

    /**
     * The byte count for a chunk in the middle is simply the chunk size.
     * \note that there may be no middle chunk if the last chunk is the
     * same as the first chunk or comes directly behind the last chunk.
     */
		inline size_t middleChunkByteCount(uint32_t chunkIndex)const {
			return _chunkSize(chunkIndex);
		}
	
		inline size_t lastChunkByteCount()const {
      const size_t cnt = chunkCount();
			if(cnt > 1) {
				const addr_t endAddress   = m_startAddress + m_byteCount;

				uint32_t chunkCount = 0;
				const size_t offset = alignedEndAddress(&chunkCount) - endAddress;
				ASSERT(chunkCount);
				const size_t chunkSize = _chunkSize(chunkCount-1);
				const size_t retval = chunkSize - offset;
				ASSERT(retval <= chunkSize);
				return retval;
			}
			return 0;
		}

    /**
     * Retrieve the number of bytes that belong to a particular chunk of the
     * Byte Array.
     */
    inline size_t getChunkByteCount(uint32_t chunkIndex)const {
      const size_t cnt = chunkCount();
      if(chunkIndex < cnt) {
        if(chunkIndex == 0) {
          return firstChunkByteCount();
        }

        if(chunkIndex == (chunkCount() - 1)) {
          return lastChunkByteCount();
        }

        return middleChunkByteCount(chunkIndex);
      }
      return 0;
    }

    /**
     * Retrieve the Byte Array address that belongs to the first
     * byte in a chunk that is not lower than the startAddress.
     */
    inline addr_t getChunkByteArrayAddress(uint32_t chunkIndex)const {
      ASSERT(!m_chunkSizeArray); /* Only supported for uniform chunks */

      if(chunkIndex < chunkCount()) {
        if(chunkIndex == 0) {
          return m_startAddress;
        }
        return alignedStartAddress() + chunkIndex * m_chunkSize;
      }
      return 0;
    }

    /**
     * Retrieve the number of bytes from startAddress until
     * an upper boundary chunk. The first byte of that upper
     * boundary chunk is not included in the count. Only
     * the bytes up to the last byte of the predecessor
     * chunk is included.
     */
    inline size_t getChunkByteOffset(uint32_t upperBoundaryChunkIndex)const {
      ASSERT(!m_chunkSizeArray); /* Only supported for uniform chunks */

      if(upperBoundaryChunkIndex > 0) {
        return firstChunkByteCount() + (upperBoundaryChunkIndex -1) * m_chunkSize;
      }
      return 0;
    }

    /**
     * Retrieve start address of a chunk. The returned value
     * may be lower than the startAddress of the Byte Array, in
     * case it is the first chunk (index 0).
     */
    inline addr_t getChunkStartAddress(uint32_t chunkIndex)const {
       if(chunkIndex < chunkCount()) {

        if(m_chunkSizeArray) {
          addr_t retval = alignedStartAddress();

          uint32_t i = 0;

          while(i < chunkIndex) {
            retval += m_chunkSizeArray->chunkSizeAt(i + m_Index0_Offset);
            i++;
          }

          return retval;

        } else {

          /* We have uniform chunks */
          return alignedStartAddress() + chunkIndex * m_chunkSize;
        }
      }
      return 0;
    }
	};
}

#endif /* UTILS_MEMORY_BLOCK_H_ */
