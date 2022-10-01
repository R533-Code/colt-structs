#ifndef HG_COLT_ITERATORS
#define HG_COLT_ITERATORS

namespace colt
{
	template<typename T>
	/// @brief Contiguous Iterator, which is just a pointer
	/// @tparam T The type of the pointer
	using ContiguousIterator = T*;
}

#endif //!HG_COLT_ITERATORS