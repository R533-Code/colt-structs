#ifndef HG_COLT_ASSERT
#define HG_COLT_ASSERT

#include "Typedefs.h"
#include <type_traits>
#include <cassert>

/// @brief Takes in a requirement and asserts it is true
#define CHECK_REQUIREMENT(NAME) assert(#NAME && NAME)

#endif //!HG_COLT_ASSERT