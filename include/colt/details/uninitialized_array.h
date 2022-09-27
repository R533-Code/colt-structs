#ifndef HG_COLT_UNINITIALIZED_ARRAY
#define HG_COLT_UNINITIALIZED_ARRAY

#include "common.h"
#include "allocator.h"

namespace colt::details
{
  template<typename T>
  class uninitialized_array
  {
    memory::TypedBlock<T> blk;


  };
}

#endif //!HG_COLT_UNINITIALIZED_ARRAY