/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */

#ifndef Utils_Object_HPP_
#define Utils_Object_HPP_

#ifdef __cplusplus

namespace utils {

/* Cannot use C++ standard type_traits because IAR Compiler below C++ 2011 standard.
 * So we do some type specific optimizations explicitly for the basic types.
 */
template <typename T> struct type_traits {
  enum {is_fundamental = false};
  static void destruct(T& object) {
    object.T::~T();
  }
};
template <> struct type_traits<char> {
  enum {is_fundamental = true};
};
template <> struct type_traits<short> {
  enum {is_fundamental = true};
};
template <> struct type_traits<int> {
  enum {is_fundamental = true};
};
template <> struct type_traits<unsigned char> {
  enum {is_fundamental = true};
};
template <> struct type_traits<unsigned int> {
  enum {is_fundamental = true};
};
template <> struct type_traits<unsigned short> {
  enum {is_fundamental = true};
};


template<typename T, typename index_type, bool is_fundamental = type_traits<T>::is_fundamental > struct object {
  typedef T object_type;
  static inline void copy(object_type* dst, const object_type* src, index_type count) {
    /* Default implementation uses the assignment operator that might be overridden by ITEM_TYPE */
    while ( count ) {
      --count;
      dst[count] = src[count];
    }
  }
};

template<typename T, typename index_type> struct object<T, index_type, true> {
  typedef T object_type;
  static inline void copy(object_type* dst, const object_type* src, index_type count) {
    /* char type uses optimized memcpy. */
    memcpy(dst, src, count * sizeof(object_type));
  }
};

} /* namespace utils */

#endif /* __cplusplus */

#endif /* Utils_Object_HPP_ */
