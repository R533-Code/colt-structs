/** @file typedefs.h
* Contains typedefs used throughout the front end.
*/

#ifndef HG_COLT_TYPEDEFS
#define HG_COLT_TYPEDEFS

#include <cstdint>

namespace colt
{
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

	/// @brief Undiscriminated union over a byte
	union BYTE
	{
		/// @brief i8
		i8 i8_v;
		/// @brief u8
		u8 u8_v;
		/// @brief bool
		bool bool_v;
		/// @brief char
		char char_v;

		constexpr BYTE() noexcept : u8_v(0) {}
		constexpr BYTE(i8 i8_v) noexcept : i8_v(i8_v) {}
		constexpr BYTE(u8 u8_v) noexcept : u8_v(u8_v) {}
		constexpr BYTE(bool bool_v) noexcept : bool_v(bool_v) {}
		constexpr BYTE(char char_v) noexcept : char_v(char_v) {}
	};

	/// @brief Undiscriminated union over a word (2-bytes)
	union WORD
	{
		/// @brief i8
		i8 i8_v;
		/// @brief u8
		u8 u8_v;
		/// @brief bool
		bool bool_v;
		/// @brief char
		char char_v;
		/// @brief BYTE
		BYTE BYTE_v;

		/// @brief i16
		i16 i16_v;
		/// @brief u16
		u16 u16_v;

		constexpr WORD() noexcept : u8_v(0) {}
		constexpr WORD(i8 i8_v) noexcept : i8_v(i8_v) {}
		constexpr WORD(u8 u8_v) noexcept : u8_v(u8_v) {}
		constexpr WORD(bool bool_v) noexcept : bool_v(bool_v) {}
		constexpr WORD(char char_v) noexcept : char_v(char_v) {}
		constexpr WORD(BYTE BYTE_v) noexcept : BYTE_v(BYTE_v) {}
		constexpr WORD(i16 i16_v) noexcept : i16_v(i16_v) {}
		constexpr WORD(u16 u16_v) noexcept : u16_v(u16_v) {}
	};

	/// @brief Undiscriminated union over a double word (4-bytes)
	union DWORD
	{
		/// @brief i8
		i8 i8_v;
		/// @brief u8
		u8 u8_v;
		/// @brief bool
		bool bool_v;
		/// @brief char
		char char_v;
		/// @brief BYTE
		BYTE BYTE_v;

		/// @brief i16
		i16 i16_v;
		/// @brief u16
		u16 u16_v;
		/// @brief WORD
		WORD WORD_v;

		/// @brief i32
		i32 i32_v;
		/// @brief u32
		u32 u32_v;
		/// @brief float
		float float_v;

		constexpr DWORD() noexcept : u32_v(0) {}
		constexpr DWORD(i8 i8_v) noexcept : i8_v(i8_v) {}
		constexpr DWORD(u8 u8_v) noexcept : u8_v(u8_v) {}
		constexpr DWORD(bool bool_v) noexcept : bool_v(bool_v) {}
		constexpr DWORD(char char_v) noexcept : char_v(char_v) {}
		constexpr DWORD(BYTE BYTE_v) noexcept : BYTE_v(BYTE_v) {}
		constexpr DWORD(i16 i16_v) noexcept : i16_v(i16_v) {}
		constexpr DWORD(u16 u16_v) noexcept : u16_v(u16_v) {}
		constexpr DWORD(WORD WORD_v) noexcept : WORD_v(WORD_v) {}
		constexpr DWORD(i32 i32_v) noexcept : i32_v(i32_v) {}
		constexpr DWORD(u32 u32_v) noexcept : u32_v(u32_v) {}
		constexpr DWORD(float float_v) noexcept : float_v(float_v) {}
	};

	/// @brief Undiscriminated union over a quadruple word (8-bytes)
	union QWORD
	{
		/// @brief i8
		i8 i8_v;
		/// @brief u8
		u8 u8_v;
		/// @brief bool
		bool bool_v;
		/// @brief char
		char char_v;
		/// @brief BYTE
		BYTE BYTE_v;

		/// @brief i16
		i16 i16_v;
		/// @brief u16
		u16 u16_v;
		/// @brief WORD
		WORD WORD_v;

		/// @brief i32
		i32 i32_v;
		/// @brief u32
		u32 u32_v;
		/// @brief float
		float float_v;
		/// @brief DWORD
		DWORD DWORD_v;

		/// @brief i64
		i64 i64_v;
		/// @brief u64
		u64 u64_v;
		/// @brief double
		double double_v;

		constexpr QWORD() noexcept : u64_v(0) {}
		constexpr QWORD(i8 i8_v) noexcept : i8_v(i8_v) {}
		constexpr QWORD(u8 u8_v) noexcept : u8_v(u8_v) {}
    constexpr QWORD(bool bool_v) noexcept : bool_v(bool_v) {}
    constexpr QWORD(char char_v) noexcept : char_v(char_v) {}
    constexpr QWORD(BYTE BYTE_v) noexcept : BYTE_v(BYTE_v) {}
    constexpr QWORD(i16 i16_v) noexcept : i16_v(i16_v) {}
    constexpr QWORD(u16 u16_v) noexcept : u16_v(u16_v) {}
    constexpr QWORD(WORD WORD_v) noexcept : WORD_v(WORD_v) {}
    constexpr QWORD(i32 i32_v) noexcept : i32_v(i32_v) {}
    constexpr QWORD(u32 u32_v) noexcept : u32_v(u32_v) {}
    constexpr QWORD(float float_v) noexcept : float_v(float_v) {}
    constexpr QWORD(DWORD DWORD_v) noexcept : DWORD_v(DWORD_v) {}
    constexpr QWORD(i64 i64_v) noexcept : i64_v(i64_v) {}
    constexpr QWORD(u64 u64_v) noexcept : u64_v(u64_v) {}
    constexpr QWORD(double double_v) noexcept : double_v(double_v) {}
	};
}

#endif //!HG_TYPEDEFS