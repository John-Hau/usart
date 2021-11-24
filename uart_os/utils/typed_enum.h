/*
 * typed_enum.h
 *
 *  Created on: 2.12.2017
 *      Author: Wolfgang
 */

#ifndef TYPED_ENUM_H_
#define TYPED_ENUM_H_

#include "baseplate.h"

#if (__cplusplus >= 201103L)
  #define TYPED_ENUM(type, name) enum name : type
  #define TYPED_ENUM_TYPEDEF(enum_type, enum_name, typedef_name) typedef enum_name typedef_name
#else
  #define TYPED_ENUM(type, name) enum name
  #define TYPED_ENUM_TYPEDEF(enum_type, enum_name, typedef_name) typedef enum_type typedef_name
#endif


#endif /* TYPED_ENUM_H_ */
