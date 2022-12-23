/** @file typedefs.h
* Contains typedefs used throughout the front end.
*/

#ifndef HG_COLT_TYPEDEFS
#define HG_COLT_TYPEDEFS

#include <cstdint>
#include <limits>

namespace colt
{
	template <class To, class From>
	inline To bit_cast(const From& src) noexcept
	{
		static_assert(sizeof(To) == sizeof(From), "sizeof of both types should be equal!");
		static_assert(std::is_trivially_copyable_v<To> && std::is_trivially_copyable_v<From>,
			"Both type should be trivially copyable!");
		static_assert(std::is_trivially_constructible_v<To>,
			"This implementation additionally requires destination type to be trivially constructible");

		To dst;
		std::memcpy(&dst, &src, sizeof(To));
		return dst;
	}	

	template<typename T>
	/// @brief Pointer
	/// @tparam T The type pointed to by the pointer
	using PTR = T*;

	/// @brief signed 8-bit integer
	using i8 = int8_t;
	/// @brief signed 16-bit integer
	using i16 = int16_t;
	/// @brief signed 32-bit integer
	using i32 = int32_t;
	/// @brief signed 64-bit integer
	using i64 = int64_t;
	/// @brief unsigned 8-bit integer
	using u8 = uint8_t;
	/// @brief unsigned 16-bit integer
	using u16 = uint16_t;
	/// @brief unsigned 32-bit integer
	using u32 = uint32_t;
	/// @brief unsigned 64-bit integer
	using u64 = uint64_t;
	/// @brief Pointer to characters
	using lstring = const char*;
	/// @brief 32-bit floating point
	using f32 = float;
	/// @brief 64-bit floating point
	using f64 = double;

	namespace traits
	{
		template<size_t sz>
		class get_uint_of_sizeof
		{
			static auto impl()
			{
				static_assert(sz == 1 || sz == 2 || sz == 4 || sz == 8);
				if constexpr (sz == 1)
					return u8{ 0 };
				else if constexpr (sz == 2)
					return u16{ 0 };
				else if constexpr (sz == 4)
					return u32{ 0 };
				else if constexpr (sz == 8)
					return u64{ 0 };
			}

		public:
			using type = decltype(impl());
		};

		template<size_t sz>
		using get_uint_of_sizeof_t = typename get_uint_of_sizeof<sz>::type;
	}

	class BYTE
	{
		u8 bits;

	public:
		constexpr BYTE() noexcept
			: bits(0) {}
		template<typename T, typename = std::enable_if_t<(sizeof(T) <= 1)>>
		constexpr BYTE(T value)
			: bits(static_cast<u8>(colt::bit_cast<traits::get_uint_of_sizeof_t<sizeof(T)>>(value))) {}

		template<typename T, typename = std::enable_if_t<(sizeof(T) <= 1)>>
		constexpr BYTE& operator=(T val) noexcept
		{
			bits = static_cast<decltype(bits)>(
				colt::bit_cast<traits::get_uint_of_sizeof_t<sizeof(T)>>(val)
				);
			return *this;
		}

		template<typename T, typename = std::enable_if_t<(sizeof(T) <= 1)>>
		constexpr T as() noexcept
		{
			return colt::bit_cast<T>(static_cast<traits::get_uint_of_sizeof_t<sizeof(T)>>(bits));
		}

		constexpr void reset_all() noexcept
		{
			bits = 0;
		}

		constexpr void set_all() noexcept
		{
			bits = std::numeric_limits<decltype(bits)>::max();
		}
	};

	class WORD
	{
		u16 bits;

	public:
		constexpr WORD() noexcept
			: bits(0) {}
		template<typename T, typename = std::enable_if_t<(sizeof(T) <= 2)>>
		constexpr WORD(T value)
			: bits(static_cast<u16>(colt::bit_cast<traits::get_uint_of_sizeof_t<sizeof(T)>>(value))) {}

		template<typename T, typename = std::enable_if_t<(sizeof(T) <= 2)>>
		constexpr WORD& operator=(T val) noexcept
		{
			bits = static_cast<decltype(bits)>(
				colt::bit_cast<traits::get_uint_of_sizeof_t<sizeof(T)>>(val)
				);
			return *this;
		}

		template<typename T, typename = std::enable_if_t<(sizeof(T) <= 2)>>
		constexpr T as() noexcept
		{
			return colt::bit_cast<T>(static_cast<traits::get_uint_of_sizeof_t<sizeof(T)>>(bits));
		}
		
		constexpr void reset_all() noexcept
		{
			bits = 0;
		}

		constexpr void set_all() noexcept
		{
			bits = std::numeric_limits<decltype(bits)>::max();
		}
	};

	class DWORD
	{
		u32 bits;

	public:
		constexpr DWORD() noexcept
			: bits(0) {}
		template<typename T, typename = std::enable_if_t<(sizeof(T) <= 4)>>
		constexpr DWORD(T value)
			: bits(static_cast<u32>(colt::bit_cast<traits::get_uint_of_sizeof_t<sizeof(T)>>(value))) {}

		template<typename T, typename = std::enable_if_t<(sizeof(T) <= 4)>>
		constexpr DWORD& operator=(T val) noexcept
		{
			bits = static_cast<decltype(bits)>(
				colt::bit_cast<traits::get_uint_of_sizeof_t<sizeof(T)>>(val)
				);
			return *this;
		}

		template<typename T, typename = std::enable_if_t<(sizeof(T) <= 4)>>
		constexpr T as() noexcept
		{
			return colt::bit_cast<T>(static_cast<traits::get_uint_of_sizeof_t<sizeof(T)>>(bits));
		}

		constexpr void reset_all() noexcept
		{
			bits = 0;
		}

		constexpr void set_all() noexcept
		{
			bits = std::numeric_limits<decltype(bits)>::max();
		}
	};

	class QWORD
	{
		u64 bits;

	public:
		constexpr QWORD() noexcept
			: bits(0) {}
		template<typename T, typename = std::enable_if_t<(sizeof(T) <= 8)>>
		constexpr QWORD(T value)
			: bits(static_cast<u64>(colt::bit_cast<traits::get_uint_of_sizeof_t<sizeof(T)>>(value))) {}

		template<typename T, typename = std::enable_if_t<(sizeof(T) <= 8)>>
		constexpr QWORD& operator=(T val) noexcept
		{
			bits = static_cast<decltype(bits)>(
				colt::bit_cast<traits::get_uint_of_sizeof_t<sizeof(T)>>(val)
				);
			return *this;
		}

		template<typename T, typename = std::enable_if_t<(sizeof(T) <= 8)>>
		constexpr T as() noexcept
		{
			return colt::bit_cast<T>(static_cast<traits::get_uint_of_sizeof_t<sizeof(T)>>(bits));
		}

		constexpr void reset_all() noexcept
		{
			bits = 0;
		}

		constexpr void set_all() noexcept
		{
			bits = std::numeric_limits<decltype(bits)>::max();
		}
	};
}

#endif //!HG_TYPEDEFS