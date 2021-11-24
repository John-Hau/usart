/*
 * osaUtils.h
 *
 *  Created on: 21.04.2013
 *      Author: Wolfgang
 */

#ifndef binary_search_H_included_
#define binary_search_H_included_

#include "baseplate.h"

namespace utils {

/**
 * Find the greatest entry in the sorted array _array_,
 * which is lower than or equal than the parameter
 * _value_
 *
 * \note if the first entry in _array is already greater
 *   than parameter _value_, then the returned entry
 *   index is -1.
 *
 * @return The index of the entry that matches the search
 *   criteria.
 */
template<typename A, typename V> ssize_t binary_search(const A& array, ssize_t arraySize, const V& value) {
  ssize_t upper_bound = arraySize - 1;
  ssize_t lower_bound = 0;
  ssize_t k = -1;

  while (lower_bound <= upper_bound) {
    k = (lower_bound + upper_bound) / 2; // choose k in the middle
    if  (array[k] == value) {
      return k; // found x
    }
    if  (array[k] > value ) {
      upper_bound = k - 1;
    }
    else {
      lower_bound = k + 1;
    }
  }
  return upper_bound;
}

}

#endif /* binary_search_H_included_ */
