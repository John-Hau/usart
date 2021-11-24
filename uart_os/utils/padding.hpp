/*
 * osaUtils.h
 *
 *  Created on: 21.04.2013
 *      Author: Wolfgang
 */

#ifndef PADDING_HPP_
#define PADDING_HPP_


#include "baseplate.h"

#pragma pack(push,1)

template<unsigned S> struct ALIGNED(1) padding
{
  unsigned char P;
  padding<S-1>  R;
  inline padding() : P(0){}
};


template<> struct ALIGNED(1) padding<1>
{
  unsigned char P;
  inline padding() : P(0){}
};

template<> struct ALIGNED(1) padding<0>
{
};

#pragma pack(pop)


constexpr unsigned PADDING_SIZE(unsigned size, unsigned alignment)
{
  return alignment - ((size + alignment - 1) % alignment) - 1;
}

/** Note: we must use an array of [(size) ? 1 : 0] here for the
 *  padding area, because an array can be of zero size, while the
 *  empty padding<0> structure still has a non zero size according
 *  to the C standard. */
#define PADD__(size) \
  padding<(size)> padding_area_[(size) ? 1 : 0];

#define PADD(all_members_size, alignment) \
  PADD__(PADDING_SIZE(all_members_size, alignment))

#endif /* PADDING_HPP_ */
