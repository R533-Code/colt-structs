#ifndef HG_COLT_ASSERT
#define HG_COLT_ASSERT

#include "Typedefs.h"
#include <type_traits>
#include <cassert>

namespace colt::debug
{
  template<typename... Args, typename = std::enable_if_t<std::conjunction_v<std::is_same<bool, Args>...>>>
  constexpr void assert_true(const char* requirement_name, Args... args) noexcept
  {
    if (!(args & ...))
      assert(false && requirement_name);
  }
}

/// @brief Takes in a requirement and asserts it is true
#define CHECK_REQUIREMENT(NAME) colt::debug::assert_true(#NAME, NAME)

#endif //!HG_COLT_ASSERT