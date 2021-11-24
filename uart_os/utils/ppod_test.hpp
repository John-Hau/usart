/*
 * ppod_test.hpp
 *
 *  Created on: 08.07.2013
 *      Author: e673505
 */

#ifndef PPOD_TEST_HPP_
#define PPOD_TEST_HPP_

#include "baseplate.h"

#if defined (PPOD_TEST)

#include "struct_member_traits.hpp"
#include "utils.h"


#include "utils/padding.hpp"
#include "utils/ppod_test.hpp"

/**
 * TEST_PPOD__(..) tests whether the layout of a PPOD structure is
 * optimized for CPU access and gives error messages at compile
 * time if not.
 *
 * Example how to apply TEST_PPOD__() on PPOD structure named X
 *
 *     template<typename CLASS> struct meta;
 *
 *     struct PPOD_ALIGNED X
 *     {
 *     #pragma pack(push,1)
 *       friend struct meta<X>;
 *
 *     private:
 *       float      A;
 *       double     B;
 *       uint16_t   D[2];
 *       int8_t     C;
 *
 *     public:
 *       enum { all_members_size = (
 *            sizeof(A)
 *          + sizeof(B)
 *          + sizeof(C)
 *          + sizeof(D)
 *          )
 *       };
 *
 *       PADD(all_members_size, PPOD_ALIGNMENT); // padd with zeroes
 *     #pragma pack(pop)
 *     };
 *
 *
 *     template<> struct meta<X>
 *     {
 *       typedef X type;
 *
 *       // Disable -Winvalid-offsetof which would appear
 *       // just because the members of X are private.
 *       #pragma GCC diagnostic ignored "-Winvalid-offsetof"
 *       typedef utils::struct_member_traits<
 *          UTILS_STRUCT_MEMBER_INFO(type, A)
 *         ,UTILS_STRUCT_MEMBER_INFO(type, B)
 *         ,UTILS_STRUCT_MEMBER_INFO(type, D)
 *         ,UTILS_STRUCT_MEMBER_INFO(type, C)
 *       > traits;
 *       #pragma GCC diagnostic warning "-Winvalid-offsetof"
 *
 *       TEST_PPOD__(traits, X, all_members_size);
 *     };
 *
 */


#define TEST_PPOD__(traits, PPOD, all_members_size_name) \
/** Test if members which are listed in the PPOD structure
 *  are all listed in struct_member_traits as well. */ \
static_assert( traits::all_members_size == PPOD::all_members_size_name, \
       "Template parameters for struct_member_traits not " \
       "matching calculation of " UTILS_TO_STRING(PPOD) "::" \
       UTILS_TO_STRING(all_members_size) ". Please fix." \
); \
/** Test if all members are defined in descending
 *  order of their size to avoid gaps respectively
 *  unaligned access by the CPU. */ \
static_assert( traits::monotonically_decreasing(alignof(PPOD)), \
       "Members of " UTILS_TO_STRING(PPOD) \
       " not defined in descending order of their size. " \
       "Please fix." \
); \
static_assert( traits::smaller_or_multiple_of(alignof(PPOD)) || \
            !traits::monotonically_decreasing(alignof(PPOD)), \
       "All members which are bigger than the alignment of " \
        UTILS_TO_STRING(PPOD) " must be a multiple of the " \
       "alignment of " UTILS_TO_STRING(PPOD) ". Please fix. " \
); \
/** If the monotonically_decreasing test and the
 *  smaller_or_multiple_of test did not fail, then the
 *  size of all members + the PPOD structure alignment
 *  padding must match the size of the PPOD structure.
 *  monotonically_decreasing and smaller_or_multiple_of
 *  assures, that there are no gaps between the members. */ \
static constexpr unsigned padding_size = \
        PADDING_SIZE(PPOD::all_members_size_name, alignof(PPOD)); \
static_assert( PPOD::all_members_size_name + padding_size==sizeof(PPOD) || \
            !traits::monotonically_decreasing(alignof(PPOD)) || \
            !traits::smaller_or_multiple_of(alignof(PPOD)), \
       "Calculation of " \
        UTILS_TO_STRING(PPOD) "::" UTILS_TO_STRING(all_members_size) \
       " is missing members. Please fix." \
)

#endif // #if defined (PPOD_TEST)

#endif /* PPOD_TEST_HPP_ */
