/*
 * unsigned_list_c.hpp
 *
 *  Created on: 30.06.2013
 *      Author: Wolfgang
 */

#ifndef UNSIGNED_LIST_C_HPP_
#define UNSIGNED_LIST_C_HPP_

#include "utils.h"
#include <limits>

namespace utils
{

/*****************************************************************************/
/** ------------------- unsigned_list_c ------------------------------------ */
/*****************************************************************************/
static constexpr unsigned uint_delimiter = std::numeric_limits<unsigned>::max();

template<unsigned int HEAD, unsigned int... FURTHER_UNSIGNED>
struct uint_list_c
{

  static constexpr auto head = HEAD;
  typedef uint_list_c<FURTHER_UNSIGNED...> further_unsigned;
  static constexpr unsigned int min = (HEAD < further_unsigned::min) ?
      HEAD : further_unsigned::min;
  static constexpr unsigned int max = (HEAD > further_unsigned::max) ?
      HEAD : further_unsigned::max;
};


template<unsigned int HEAD>
struct uint_list_c<HEAD>
{
  static constexpr auto head = HEAD;
  static constexpr auto tail = uint_delimiter;
  static constexpr auto min = HEAD;
  static constexpr auto max = HEAD;
};


/*****************************************************************************/
/** ------------------- unsigned_odd_array_c ------------------------------- */
/*****************************************************************************/

// unsigned_odd_array_c__ is a helper struct for unsigned_odd_array_c
template<bool IS_ODD, unsigned int HEAD_VALUE, unsigned int... FURTHER_VALUES>
struct unsigned_odd_array_c__;

template<unsigned int HEAD_VALUE, unsigned int... FURTHER_VALUES>
struct unsigned_odd_array_c__<true, HEAD_VALUE, FURTHER_VALUES...>
{
  typedef typename utils::uint_list_c<FURTHER_VALUES...> rest;
  unsigned int hv;
  unsigned_odd_array_c__<(rest::head % 2) != 0, FURTHER_VALUES...> fv;
  constexpr unsigned_odd_array_c__() : hv(HEAD_VALUE) {}
};

template<unsigned int HEAD_VALUE, unsigned int... FURTHER_VALUES>
struct unsigned_odd_array_c__<false, HEAD_VALUE, FURTHER_VALUES...>
{
  typedef typename utils::uint_list_c<FURTHER_VALUES...> rest;
  unsigned_odd_array_c__<(rest::head % 2) != 0, FURTHER_VALUES...> fv;
};

template<unsigned int HEAD_VALUE>
struct unsigned_odd_array_c__<true, HEAD_VALUE>
{
  unsigned int hv;
  unsigned int delimiter;
  constexpr unsigned_odd_array_c__() : hv(HEAD_VALUE), delimiter(utils::uint_delimiter) {}
};

template<unsigned int HEAD_VALUE>
struct unsigned_odd_array_c__<false, HEAD_VALUE>
{
  unsigned int delimiter;
  constexpr unsigned_odd_array_c__() : delimiter(utils::uint_delimiter) {}
};


/** unsigned_odd_array provides a const array containing only the odd values
 *  of a list of unsigned int template parameters.
 *
 *  Example:
 *   typedef utils::unsigned_odd_array_c<2, 13, 7, 8 , 5> X;
 *   const unsigned int* array = X::value; // becomes { 13, 7, 5 };
 *   unsigned array_size = X::size;        // becomes 3
 */
template<unsigned int HEAD_VALUE, unsigned int... FURTHER_VALUES>
struct unsigned_odd_array_c
{
#ifndef _DEBUG
private:
#endif
  typedef unsigned_odd_array_c__<(HEAD_VALUE % 2) != 0,  HEAD_VALUE, FURTHER_VALUES...> type;
  static const type data_;

private:
  /** make constructor private to avoid the creation of an object */
  inline unsigned_odd_array_c() {}

public:
  /* Note: size excludes the delimiter at the end of the array */
  static constexpr const unsigned int* value = reinterpret_cast<const unsigned int*>(&data_);
  static constexpr unsigned size = ( sizeof(type) / sizeof(unsigned int) ) - 1;
};

template<unsigned int HEAD_VALUE, unsigned int... FURTHER_VALUES>
const typename unsigned_odd_array_c<HEAD_VALUE, FURTHER_VALUES...>::type unsigned_odd_array_c<HEAD_VALUE, FURTHER_VALUES...>::data_;


/*****************************************************************************/
/** ------------------- unsigned_even_array_c ------------------------------- */
/*****************************************************************************/

// unsigned_even_array_c__ is a helper struct for unsigned_even_array_c
template<bool IS_EVEN, unsigned int HEAD_VALUE, unsigned int... FURTHER_VALUES>
struct unsigned_even_array_c__;

template<unsigned int HEAD_VALUE, unsigned int... FURTHER_VALUES>
struct unsigned_even_array_c__<true, HEAD_VALUE, FURTHER_VALUES...>
{
  typedef typename utils::uint_list_c<FURTHER_VALUES...> rest;
  unsigned int hv;
  unsigned_even_array_c__<(rest::head % 2) == 0, FURTHER_VALUES...> fv;
  constexpr unsigned_even_array_c__() : hv(HEAD_VALUE) {}
};

template<unsigned int HEAD_VALUE, unsigned int... FURTHER_VALUES>
struct unsigned_even_array_c__<false, HEAD_VALUE, FURTHER_VALUES...>
{
  typedef typename utils::uint_list_c<FURTHER_VALUES...> rest;
  unsigned_even_array_c__<(rest::head % 2) == 0, FURTHER_VALUES...> fv;
};

template<unsigned int HEAD_VALUE>
struct unsigned_even_array_c__<true, HEAD_VALUE>
{
  unsigned int hv;
  unsigned int delimiter;
  constexpr unsigned_even_array_c__() : hv(HEAD_VALUE), delimiter(utils::uint_delimiter) {}
};

template<unsigned int HEAD_VALUE>
struct unsigned_even_array_c__<false, HEAD_VALUE>
{
  unsigned int delimiter;
  constexpr unsigned_even_array_c__() : delimiter(utils::uint_delimiter) {}
};


/** unsigned_even_array_c provides a const array containing only the even values
 *  of a list of unsigned int template parameters.
 *
 *  Example:
 *   typedef utils::unsigned_odd_array_c<2, 13, 7, 8 , 5> X;
 *   const unsigned int* array = X::value; // becomes { 2, 8 };
 *   unsigned array_size = X::size;        // becomes 2
 */
template<unsigned int HEAD_VALUE, unsigned int... FURTHER_VALUES>
struct unsigned_even_array_c
{
#ifndef _DEBUG
private:
#endif
  typedef unsigned_even_array_c__<(HEAD_VALUE % 2) == 0,  HEAD_VALUE, FURTHER_VALUES...> type;
  static const type data_;

private:
  /** make constructor private to avoid the creation of an object */
  inline unsigned_even_array_c() {}

public:
  /* Note: size excludes the delimiter at the end of the array */
  static constexpr const unsigned int* value = reinterpret_cast<const unsigned int*>(&data_);
  static constexpr unsigned size = ( sizeof(type) / sizeof(unsigned int) ) - 1;
};

template<unsigned int HEAD_VALUE, unsigned int... FURTHER_VALUES>
const typename unsigned_even_array_c<HEAD_VALUE, FURTHER_VALUES...>::type unsigned_even_array_c<HEAD_VALUE, FURTHER_VALUES...>::data_;

}

#endif /* UNSIGNED_LIST_C_HPP_ */
