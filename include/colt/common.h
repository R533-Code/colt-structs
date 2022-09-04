#ifndef HG_COLT_COMMON
#define HG_COLT_COMMON

#include <cassert>

/// @brief Contains all colt provided utilities
namespace colt {	
	
	/// @brief Represents a size in bytes
	struct ByteSize
	{
		/// @brief The byte size
		size_t size;
	};

	/// @brief User defined literal to convert a value to a ByteSize
	/// @param size The size to convert
	/// @return ByteSize of size 'size'
	constexpr ByteSize operator"" _B(size_t size) noexcept { return { size }; }

	/// @brief Represents a size in kilo-bytes
	struct KiloByteSize
	{
		/// @brief The kilo-byte size
		size_t size;

		/// @brief Conversion operator to ByteSize
		/// @return ByteSize
		constexpr operator ByteSize() const noexcept { return { size * 1024 }; }
	};

	/// @brief User defined literal to convert a value to a KiloByteSize
	/// @param size The size to convert
	/// @return KiloByteSize of size 'size'
	constexpr KiloByteSize operator"" _KB(size_t size) noexcept { return { size }; }

	/// @brief Represents a size in mega-bytes
	struct MegaByteSize
	{
		/// @brief The mega-byte size
		size_t size;
		
		/// @brief Conversion operator to KiloByteSize
		/// @return KiloByteSize
		constexpr operator KiloByteSize() const noexcept { return { size * 1024 }; }
		/// @brief Conversion operator to ByteSize
		/// @return ByteSize
		constexpr operator ByteSize() const noexcept { return { size * 1024 * 1024 }; }
	};

	/// @brief User defined literal to convert a value to a MegaByteSize
	/// @param size The size to convert
	/// @return MegaByteSize of size 'size'
	constexpr MegaByteSize operator"" _MB(size_t size) noexcept { return { size }; }

	/// @brief Represents a size in giga-bytes
	struct GigaByteSize
	{
		/// @brief The giga-byte size
		size_t size;

		/// @brief Conversion operator to MegaByteSize
		/// @return MegaByteSize
		explicit constexpr operator MegaByteSize() const noexcept { return { size * 1024 }; }
		/// @brief Conversion operator to KiloByteSize
		/// @return KiloByteSize
		explicit constexpr operator KiloByteSize() const noexcept { return { size * 1024 * 1024 }; }
		/// @brief Conversion operator to ByteSize
		/// @return ByteSize
		explicit constexpr operator ByteSize() const noexcept { return { size * 1024 * 1024 * 1024 }; }
	};
	
	/// @brief User defined literal to convert a value to a GigaByteSize
	/// @param size The size to convert
	/// @return GigaByteSize of size 'size'
	constexpr ByteSize operator"" _GB(size_t size) noexcept { return { size }; }
}

#endif //!HG_COLT_COMMON