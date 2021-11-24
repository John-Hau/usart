/*
 * osaUtils.h
 *
 *  Created on: 21.04.2013
 *      Author: Wolfgang
 */

#ifndef INT_HPP_
#define INT_HPP_

#include <stdint.h>

#ifdef __cplusplus
 // Retrieve a signed integer type based on the size in bytes
 // --- signed int ---
 template<unsigned SIZE> struct signed_int {
 };

 template<> struct signed_int<1> {
    typedef int8_t type;
 };

 template<> struct signed_int<2> {
    typedef int16_t type;
 };

 template<> struct signed_int<4> {
    typedef int32_t type;
 };

 template<> struct signed_int<8> {
    typedef int64_t type;
 };

// Retrieve an unsigned integer type based on the size in bytes
 // --- unsigned int ---
 template<unsigned SIZE> struct unsigned_int {
 };

 template<> struct unsigned_int<1> {
    typedef uint8_t type;
 };

 template<> struct unsigned_int<2> {
    typedef uint16_t type;
 };

 template<> struct unsigned_int<4> {
    typedef uint32_t type;
 };

 template<> struct unsigned_int<8> {
    typedef uint64_t type;
 };

 /* Choose the type with the smaller size */
 template<typename A, typename B, bool AsmallerB = (sizeof(A) > sizeof(B))>
 struct min_size_type {
   typedef A type;
 };

 template<typename A, typename B>
 struct min_size_type<A, B, false> {
   typedef B type;
 };

namespace utils {
 /* Logarithmus to base 2 of integers which are power of 2 */
 template<unsigned x>struct log2 {
   /* Not supported for any other than specialized values. */
 };

 template<>struct log2<1> {
   enum {value = 0};
 };

 template<>struct log2<2> {
   enum {value = 1};
 };

 template<>struct log2<4> {
   enum {value = 2};
 };

 template<>struct log2<8> {
   enum {value = 3};
 };
} /* namespace utils */

#endif /* __cplusplus */

#endif /* INT_HPP_ */
