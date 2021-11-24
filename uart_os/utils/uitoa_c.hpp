/*
 * osaUtils.h
 *
 *  Created on: 21.04.2013
 *      Author: Wolfgang
 */

#ifndef UITOA_C_HPP_
#define UITOA_C_HPP_

#if __cplusplus >= 201103

namespace utils
{

#pragma pack(push,1)

// uitoa_c__ is a helper struct for uitoa_c
template <unsigned O, unsigned N, unsigned R, char... prefix > struct uitoa_c__;

template <unsigned O, unsigned N, unsigned R, char C, char... prefix >
struct uitoa_c__<O, N, R, C, prefix... > {
    char c;
    uitoa_c__<O, N, R, prefix...> r;
    uitoa_c__() : c(C) {}
};

template <unsigned O, unsigned N, unsigned R >
struct uitoa_c__<O, N, R > {
    uitoa_c__<O, R, (R/10)> x;
    char r;
    uitoa_c__() : r('0' + (N % 10)) {}
};

template <unsigned O, unsigned R >
struct uitoa_c__<O, O, R > {
    uitoa_c__<O, R, (R/10)> x;
    char r;
    char t;
    uitoa_c__() : r('0' + (O % 10)), t('\0') {}

};

template <unsigned O >
struct uitoa_c__<O, O, 0 > {
    char r;
    char t;
    uitoa_c__() : r('0' + (O % 10)), t('\0') {}
};

template <unsigned O, unsigned N>
struct uitoa_c__ <O, N, 0> {
    char r;
    uitoa_c__() : r('0' + N) {}

#pragma pack(pop)

};


/** uitoa_c provides a compile time const string for an unsigned integer.
 *  The resulting string is preceded by an optional prefix string.
 *
 *  Example:
 *
 *  typedef utils::uitoa_c<12345, 'h','e', 'l', 'l', 'o', '_'> X;
 *
 *    X::value; // -> "hello_12345"
 *    X::size;  // ->  11
 *
 *  */
template<unsigned UI, char... prefix>
struct uitoa_c
{
private:
  typedef uitoa_c__<UI, UI, UI/10, prefix...> type;
  static const type data_;

  /** make constructor private to avoid the creation of an object */
  inline uitoa_c() {}

public:
  /* Note: size excludes the \0 at the end of the string */
  static constexpr const char* value = reinterpret_cast<const char*>(&data_);
  static constexpr unsigned size = (sizeof(type) / sizeof(char) ) - 1;
};

template<unsigned I, char... prefix>
const typename uitoa_c<I, prefix...>::type uitoa_c<I, prefix...>::data_;

} /** namespace utils */

#endif /* #if __cplusplus >= 201103 */

#endif /* UITOA_C_HPP_ */
