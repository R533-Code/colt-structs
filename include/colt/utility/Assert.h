/** @file Assert.h
* Contains assertion helpers.
* While the idea was to provide better preconditions through macros,
* I wasn't able to create any non-intrusive/useful stuff.
* This header (for now) only contains CHECK_REQUIREMENT
* which expects an expression to assert as true.
*/

#ifndef HG_COLT_ASSERT
#define HG_COLT_ASSERT

#include "Typedefs.h"
#include <type_traits>
#include <cassert>

/// @brief Takes in a requirement and asserts it is true
#define CHECK_REQUIREMENT(NAME) assert(#NAME && NAME)

#endif //!HG_COLT_ASSERT