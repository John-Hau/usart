/*
 * struct_member_traits.hpp
 *
 *  Created on: 07.07.2013
 *      Author: Wolfgang
 */

#ifndef STRUCT_MEMBER_TRAITS_HPP_
#define STRUCT_MEMBER_TRAITS_HPP_

#include <stddef.h>
#include <type_traits>

namespace utils
{



/*****************************************************************************/
/** ------------------- struct_member_traits ------------------------------- */
/*****************************************************************************/

/** struct_member_info<SIZE, OFFS>
 * are the MEMBER_INFO template parameters to be passed to
 * struct_member_traits<> */
template<unsigned OFFS, typename TYPE>
struct struct_member_info
{
  typedef TYPE type;
  typedef typename std::remove_all_extents<type>::type element_type;

  static constexpr unsigned offs = OFFS; // offset of the structure member
  static constexpr unsigned size = sizeof(type); // size   of the structure member
  static constexpr unsigned element_size = sizeof(element_type);
};


#define UTILS_STRUCT_MEMBER_INFO(struct_name, member_name) \
    utils::struct_member_info<offsetof(struct_name,member_name), \
    decltype(struct_name::member_name)>


/** struct_member_traits provides a information about members of a structure.
 *
 *   -) monotonically_decreasing:
 *       tests, whether the members of the structure are ordered by their size
 *       from big to small if they are smaller than 'alignment'. Returns true
 *       if this test succeeds, otherwise false.
 *       If the test succeeds, it assures processor optimized access for packed
 *       structures, respectively size optimized structure for non-packed
 *       structures.
 *       Note: the template parameters can be passed in random order.
 *
 *   -) all_members_size:
 *       calculates the size of all members which might be different to the
 *       size of the structure in case of a byte alignment which is greater
 *       than 1.
 *
 *       Example:
 *
 *          struct X
 *          {
 *            unsigned int   A;
 *            unsigned char  B;
 *            unsigned short C;
 *          };
 *
 *          typedef utils::struct_member_traits<
 *                 UTILS_STRUCT_MEMBER_INFO(X::A)
 *                ,UTILS_STRUCT_MEMBER_INFO(X::C)>
 *                ,UTILS_STRUCT_MEMBER_INFO(X::B)>
 *              > X_traits
 *
 *          static_assert(
 *              X_traits::monotonically_decreasing,
 *              "Error: Members in descending order of their size."
 *          );
 *
 *          const sX = X_traits::all_members_size;
 *
*/
template<typename MEMBER_INFO, typename... FURTHER_MEMBER_INFOS>
struct struct_member_traits; // Forward declaration

namespace struct_member_traits__
{


template<typename MEMBER_INFO, typename SECOND_MEMBER_INFO, typename... FURTHER_MEMBER_INFOS>
struct remove_ssecond
{
  typedef struct_member_traits<MEMBER_INFO, FURTHER_MEMBER_INFOS...> result;
};

template<typename MEMBER_INFO, typename SECOND_MEMBER_INFO>
struct remove_ssecond<MEMBER_INFO, SECOND_MEMBER_INFO>
{
  typedef struct_member_traits<MEMBER_INFO> result;

};

} /** namespace struct_member_traits__ */


template<typename MEMBER_INFO, typename... FURTHER_MEMBER_INFOS>
struct struct_member_traits
{
  typedef MEMBER_INFO head;
  typedef struct_member_traits<FURTHER_MEMBER_INFOS...> rest;

  static constexpr bool both_greater_ore_equal_than_align(unsigned alignment)
  {
    return (head::element_size >= alignment) && (rest::head::element_size >= alignment);
  }

#ifdef DEBUG_struct_member_traits
  static constexpr bool head_and_rest_of_rest_greater_ore_equal_than_align(unsigned alignment)
  {
    return remove_ssecond<head, FURTHER_MEMBER_INFOS...>::both_greater_ore_equal_than_align(alignment);
  }
#endif

  /** Test whether the size (element_size in case of arrays) of
   *  the structure members decrease with their offset if they
   *  are smaller than 'alignment'. This assures processor
   *  optimized access for packed structures, respectively size
   *  optimization for non-packed structures. */
  static constexpr bool monotonically_decreasing(unsigned alignment)
  {
    typedef typename struct_member_traits__::remove_ssecond<head, FURTHER_MEMBER_INFOS...>::result remove_second;
    return (
        // Test head against head of rest
        (
            // don't care about the order if head and rest::head
            // are greater of equal than alignment
            both_greater_ore_equal_than_align(alignment)
               ||
            // care about the order if either is smaller than alignment
            (
              head::offs > rest::head::offs ?
                (head::element_size <= rest::head::element_size) :
                (head::element_size >= rest::head::element_size)
            )
        )
        // Test head against 3rd and further member infos
        && remove_second::monotonically_decreasing(alignment)
        // Test rest against each other
        && rest::monotonically_decreasing(alignment)
    );
  }

  /** Calculate the sum of the size of all members */
  static constexpr unsigned all_members_size = head::size + rest::all_members_size;

  /** Test whether size (element_size in case of arrays) all members
   *  are smaller than alignment or a multiple of alignment */
  static constexpr bool smaller_or_multiple_of(unsigned alignment)
  {
    return (
            (head::element_size <= alignment) || ((head::element_size % alignment) == 0)
        ) && rest::smaller_or_multiple_of(alignment);
  }
};


template<typename MEMBER_INFO>
struct struct_member_traits<MEMBER_INFO>
{
  typedef MEMBER_INFO head;

  static constexpr bool both_greater_ore_equal_than_align(unsigned alignment)
  {
    return (head::element_size >= alignment);
  }

  static constexpr bool monotonically_decreasing(unsigned UNUSED(alignment)) {return true;}
  static constexpr unsigned all_members_size = head::size;

  static constexpr bool smaller_or_multiple_of(unsigned alignment) {
      return (head::element_size <= alignment) || ((head::element_size % alignment) == 0);
  }
};


} /** namespace utils */



#endif /* STRUCT_MEMBER_TRAITS_HPP_ */
