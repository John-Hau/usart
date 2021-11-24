/*
 *  $HeadURL: $
 */


#ifndef _custom_stl_allocator_H_INCLUDED_
#define _custom_stl_allocator_H_INCLUDED_


namespace custom_stl {

enum E_MALLOC_FASHION {
  MF_TRACED = -2,
  MF_NORMAL = -1,
  MF_FIRST_CUSTOM = 0 /* Never change this! */
};

/**
 *
 * _malloc_fashion_ is used by the custom_stl::allocator<typename T, ..> in order to
 * allocate respectively deallocate memory for instances of type T.
 *
 * The default implementation just uses C standard heap functions
 * ::malloc and ::free.
 *
 * __Code Example:__
 ~~~~~~~~{.cpp}
 // Apply custom_stl::allocator<> that uses the default implementation
 // of malloc_fashion. In this example for a vector of uint16_t:
 std::vector<uint16_t, custom_stl::allocator<uint16_t>
 ~~~~~~~~
 *
 *
 * Beyond the default implementation there is a specialized implementation available
 * that utilizes a MallocTracer structure for memory allocation respectively
 * deallocation. If you want to use this implementation, you need to pass MF_TRACED
 * as an additional template parameter to the allocator class.
 *
 * __Code Example:__
 ~~~~~~~~{.cpp}
 // Apply custom_stl::allocator<> that uses the MallocTracer implementation
 // of malloc_fashion. In this example for a vector of uint16_t:
 std::vector<uint16_t, custom_stl::allocator<uint16_t, custom_stl::MF_TRACED>

 // You may want to access the mallocTracer object being used by this allocator:
 MallocTracer* mt = &custom_stl::malloc_fashion<custom_stl::MF_TRACED>::mallocTracer;
 unsigned count = mt->mallocCount();
 unsigned size = mt->mallocSize();
 ~~~~~~~~
 *
 * You may want to specialize the malloc_fashion<MF_TRACED> even more.
 * This can be done by passing a type for the MALLOC_CONTEXT template parameter
 * to class custom_stl::allocator. The MALLOC_CONTEXT is intended to provide
 * more information about the instantiated STL container.
 *
 * Assume you have the following code:
 *
 * std::set<uint16_t, custom_stl::allocator<uint16_t, custom_stl::MF_TRACED> gVar;
 *
 * class uint16Container0 {
 *  std::vector<uint16_t, custom_stl::allocator<uint16_t, custom_stl::MF_TRACED> myVar;
 * };
 *
 * class uint16Container1 {
 *  std::vector<uint16_t, custom_stl::allocator<uint32_t, custom_stl::MF_TRACED> myVar;
 *  std::list<uint16_t, custom_stl::allocator<uint8_t, custom_stl::MF_TRACED> myVar2;
 * };
 *
 * In that case, all containers will use the same malloc_fashion. Hence you cannot distinguish how much
 * memory was allocated for what.
 * MallocTracer* mt = &custom_stl::malloc_fashion<custom_stl::MF_TRACED>::mallocTracer;
 * unsigned size = mt->mallocSize();
 *
 * This will retrieve the aggregated memory allocation size for
 * all the above containers: gVar, instances of uint16Container0::myVar, instances of
 * uint16Container1::myVar and uint16Container1::myVar2.
 *
 * You may want to distinguish the memory that is allocated just by instances of uint16Container2.
 * That can done by passing a MALLOC_CONTEXT parameter to custom_stl::allocator. This will
 * generate an own malloc_fashion with an own mallocTracer object:
 *
 * class uint16Container1 {
 *  std::vector<uint16_t, custom_stl::allocator<uint32_t, custom_stl::MF_TRACED, class uint16Container1> myVar;
 *  std::list<uint16_t, custom_stl::allocator<uint8_t, custom_stl::MF_TRACED, class uint16Container1> myVar2;
 * };
 *
 * Now the allocated memory for instances of uint16Container0::myVar, instances of
 * uint16Container1::myVar and uint16Container1::myVar2 are separated into a separate malloc_fashion object:
 *
 * MallocTracer* mt = &custom_stl::malloc_fashion<custom_stl::MF_TRACED, class uint16Container1>::mallocTracer;
 * unsigned size = mt->mallocSize();
 *
 * In the above example we use uint16Container1 as the MALLOC_CONTEXT parameter. However this is not mandatory.
 * You can use any other type for separation, and this type does event not need to exist. See below code
 * will do the same as before:
 *
 * class uint16Container1 {
 *  std::vector<uint16_t, custom_stl::allocator<uint32_t, custom_stl::MF_TRACED, struct dummy> myVar;
 *  std::list<uint16_t, custom_stl::allocator<uint8_t, custom_stl::MF_TRACED, struct dummy> myVar2;
 * };
 * MallocTracer* mt = &custom_stl::malloc_fashion<custom_stl::MF_TRACED, struct dummy>::mallocTracer;
 * unsigned size = mt->mallocSize();
 *
 * This concept allows to further distinguish between uint16Container1::myVar and uint16Container1::myVar2:
 * class uint16Container1 {
 *  std::vector<uint16_t, custom_stl::allocator<uint32_t, custom_stl::MF_TRACED, struct dummy> myVar;
 *  std::list<uint16_t, custom_stl::allocator<uint8_t, custom_stl::MF_TRACED, struct dummy2> myVar2;
 * };
 * MallocTracer* mt = &custom_stl::malloc_fashion<custom_stl::MF_TRACED, struct dummy>::mallocTracer;
 * unsigned size = mt->mallocSize(); // size of all instances of uint16Container1::myVar;
 * MallocTracer* mt2 = &custom_stl::malloc_fashion<custom_stl::MF_TRACED, struct dummy2>::mallocTracer;
 * unsigned size2 = mt2->mallocSize(); // size of all instances of uint16Container1::myVar2;
 *
 *
 * If you don't want to use any of the above implementations then you may
 * specialize malloc_fashion for a particular _MALLOC_FASHION_SELECTOR_ by yourself.
 * Then you need to pass the _MALLOC_FASHION_SELECTOR_ of your specialization to
 * the allocator that you pass to the STL container.
 *
 * __Code Example:__
 ~~~~~~~~{.cpp}
 // Specialize malloc_fashion for MALLOC_FASHION_SELECTOR = 5
 namespace custom_stl {

 template<typename MALLOC_CONTEXT> struct malloc_fashion<5, MALLOC_CONTEXT> {
   static void* malloc_(std::size_t size) {
     // ... your implementation.
   }

   static void free_(void* mem) {
     // ... your implementation.
   }

   static int myExampleVariable;
 };

 template<typename MALLOC_CONTEXT> int malloc_fashion<5, MALLOC_CONTEXT>::myExampleVariable;

 }

 // Apply custom_stl::allocator<> that uses the specialized
 // malloc_fashion with MALLOC_FASHION_SELECTOR = 5. In this example for a vector of uint16_t:
 std::vector<uint16, custom_stl::allocator<uint16, 5>

 // You may want to access your myExampleVariable of your specialized malloc_fashion:
 int v = custom_stl::malloc_fashion<5>.myExampleVariable;
 ~~~~~~~~
 *
 *
 * Furthermore you may specialize malloc_fashion for particular _MALLOC_FASHION_SELECTOR_ and _MALLOC_CONTEXT_.
 * Then you need to pass the _MALLOC_FASHION_SELECTOR_ and _MALLOC_CONTEXT_ of your specialization to
 * the allocator that you pass to the STL container.
 *
 * __Code Example:__
 ~~~~~~~~{.cpp}
 // Specialize malloc_fashion for MALLOC_FASHION_SELECTOR = 5 and MALLOC_CONTEXT = struct myContext
 namespace custom_stl {

 template<> struct malloc_fashion<5, struct myContext> {
   static void* malloc_(std::size_t size) {
     // ... your implementation.
   }

   static inline void free_(void* mem) {
     // ... your implementation.
   }

   static int myExampleVariable;
 };

 template<> int malloc_fashion<5, struct myContext>::myExampleVariable;

 }

 // Apply custom_stl::allocator<> that uses the specialized
 // malloc_fashion with MALLOC_FASHION_SELECTOR = 5  and MALLOC_CONTEXT = struct myContext.
 // In this example for a vector of uint16_t:
 std::vector<uint16, custom_stl::allocator<uint16, 5, struct myContext>

 // You may want to access your myExampleVariable of your specialized malloc_fashion:
 int v = custom_stl::malloc_fashion<5, struct myContext>.myExampleVariable;
 ~~~~~~~~
 *
 */
template<int MALLOC_FASHION_SELECTOR, typename MALLOC_CONTEXT = struct UNKNOWN_MALLOC_CONTEXT>
struct malloc_fashion {

  static inline void* malloc_(std::size_t size) {
    return ::malloc(size);
  }

  static inline void free_(void* mem) {
    ::free(mem);
  }
};

template<typename T>
class object_traits
{
public:

  typedef T type;

  template<typename U>
  struct rebind
  {
    typedef object_traits<U> other;
  };

  // Constructor
  object_traits(void) {
  }

  // Copy Constructor
  template<typename U>
  object_traits(object_traits<U> const& other) {
  }

  // Address of object
  type* address(type& obj) const {
    return &obj;
  }
  type const* address(type const& obj) const {
    return &obj;
  }

  // Construct object
  void construct(type* ptr, type const& ref) const
    {
    // In-place copy construct
    new (ptr) type(ref);
  }

  // Destroy object
  void destroy(type* ptr) const
    {
    // Call destructor
    ptr->~type();
  }
};

#define CUSTOM_STL_ALLOCATOR_TRAITS(T)       \
  typedef T                 type;            \
  typedef type              value_type;      \
  typedef value_type*       pointer;         \
  typedef value_type const* const_pointer;   \
  typedef value_type&       reference;       \
  typedef value_type const& const_reference; \
  typedef std::size_t       size_type;       \
  typedef std::ptrdiff_t    difference_type; \

template<typename T>
struct max_allocations
{
  enum {
    value = static_cast<std::size_t>(-1) / sizeof(T)
  };
};

template<typename T, int MALLOC_FASHION_SELECTOR, typename MALLOC_CONTEXT>
class heap
{
public:

  CUSTOM_STL_ALLOCATOR_TRAITS(T)

  template<typename U>
  struct rebind
  {
    typedef heap<U, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT> other;
  };

  // Default Constructor
  heap(void) {
  }

  // Copy Constructor
  template<typename U>
  heap(heap<U, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT> const& other) {
  }

  // Allocate memory
  pointer allocate(size_type count, const_pointer /* hint */= 0)
    {
//    if(count > max_size()){throw std::bad_alloc();}
    return static_cast<pointer>(malloc_fashion<MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT>::malloc_(count * sizeof(type)));
  }

  // Delete memory
  void deallocate(pointer ptr, size_type /* count */)
    {
    malloc_fashion<MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT>::free_(ptr);
  }

  // Max number of objects that can be allocated in one call
  size_type max_size(void) const {
    return max_allocations<T>::value;
  }
};


#define CUSTOM_STL_FORWARD_ALLOCATOR_TRAITS(C)         \
  typedef typename C::value_type      value_type;      \
  typedef typename C::pointer         pointer;         \
  typedef typename C::const_pointer   const_pointer;   \
  typedef typename C::reference       reference;       \
  typedef typename C::const_reference const_reference; \
  typedef typename C::size_type       size_type;       \
  typedef typename C::difference_type difference_type; \


template<
  typename T, int MALLOC_FASHION_SELECTOR = MF_NORMAL, typename MALLOC_CONTEXT = struct UNKNOWN_MALLOC_CONTEXT,
  typename PolicyT = heap<T, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT>,
  typename TraitsT = object_traits<T>
  >
class allocator : public PolicyT, public TraitsT
{
public:

  // Template parameters
  typedef PolicyT Policy;
  typedef TraitsT Traits;

  CUSTOM_STL_FORWARD_ALLOCATOR_TRAITS(Policy)

template  <typename U>
  struct rebind
  {
    typedef allocator<
      U, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT,
      typename Policy::template rebind<U>::other,
      typename Traits::template rebind<U>::other
    > other;
  };

  // Constructor
  allocator(void) {}

  // Copy Constructor
  template<
    typename U, typename PolicyU, typename TraitsU
  >
  allocator(allocator<U, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT, PolicyU, TraitsU> const& other)
    :  Policy(other), Traits(other)
  {}
};

// Two allocators are not equal unless a specialization says so
template<
  typename T, typename PolicyT, typename TraitsT,
  typename U, typename PolicyU, typename TraitsU,
  int MALLOC_FASHION_SELECTOR, typename MALLOC_CONTEXT
  >
bool operator==(
  allocator<T, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT, PolicyT, TraitsT> const& left,
  allocator<U, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT, PolicyU, TraitsU> const& right
  ) {
  return false;
}

// Also implement inequality
template<
  typename T, typename PolicyT, typename TraitsT,
  typename U, typename PolicyU, typename TraitsU,
  int MALLOC_FASHION_SELECTOR, typename MALLOC_CONTEXT
  >
bool operator!=(
  allocator<T, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT, PolicyT, TraitsT> const& left,
  allocator<U, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT, PolicyU, TraitsU> const& right
  ) {
  return !(left == right);
}

// Comparing an allocator to anything else should not show equality
template<
  typename T, typename PolicyT, typename TraitsT,
  typename OtherAllocator, int MALLOC_FASHION_SELECTOR, typename MALLOC_CONTEXT
  >
bool operator==(
  allocator<T, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT, PolicyT, TraitsT> const& left,
  OtherAllocator const& right
  )
  {
  return false;
}

// Also implement inequality
template<
  typename T, typename PolicyT, typename TraitsT,
  typename OtherAllocator, int MALLOC_FASHION_SELECTOR, typename MALLOC_CONTEXT
  >
bool operator!=(
  allocator<T, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT, PolicyT, TraitsT> const& left,
  OtherAllocator const& right
  )
  {
  return !(left == right);
}

// Specialize for the heap policy
template<
  typename T, typename TraitsT,
  typename U, typename TraitsU,
  int MALLOC_FASHION_SELECTOR, typename MALLOC_CONTEXT
  >
bool operator==(
  allocator<T, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT, heap<T, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT>, TraitsT> const& left,
  allocator<U, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT, heap<U, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT>, TraitsU> const& right
  ) {
  return true;
}

// Also implement inequality
template<
  typename T, typename TraitsT,
  typename U, typename TraitsU,
  int MALLOC_FASHION_SELECTOR, typename MALLOC_CONTEXT
  >
bool operator!=(
  allocator<T, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT, heap<T, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT>, TraitsT> const& left,
  allocator<U, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT, heap<U, MALLOC_FASHION_SELECTOR, MALLOC_CONTEXT>, TraitsU> const& right
  )
  {
  return !(left == right);
}

} /* namespace custom_stl */


#include "utils/mallocTracer.hpp"

namespace custom_stl {

/**
 * malloc_fashion specialization for MF_TRACED
 */
template<typename MALLOC_CONTEXT>
struct malloc_fashion<MF_TRACED, MALLOC_CONTEXT> {
  static struct MallocTracer mallocTracer;

  static inline void* malloc_(std::size_t size) {
    return mallocTracer.m(size);
  }

  static inline void free_(void* mem) {
    mallocTracer.f(mem);
  }
};

template<typename MALLOC_CONTEXT> struct MallocTracer malloc_fashion<MF_TRACED, MALLOC_CONTEXT>::mallocTracer;

} /* namespace custom_stl */


#endif /* #ifndef _custom_stl_allocator_H_INCLUDED_ */
