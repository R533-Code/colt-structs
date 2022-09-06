#ifndef HG_COLT_COMMON
#define HG_COLT_COMMON

#include <cassert>
#include <cstring>

#include <type_traits>
#include <limits>
#include <memory>

/// @brief Generates the template meta-programming boilerplate to check if a type has a member.
/// Usage: COLT_HAS_MEMBER(print)
/// has_print<int>::value || has_print_v<int>
#define COLT_HAS_MEMBER(member) template<typename T>\
	class has_##member \
	{	\
		using one = char; \
		struct two {char x[2]; }; \
		template <typename C> static one test(decltype(&C::##member)); \
		template <typename C> static two test(...); \
	public: \
		enum { value = sizeof(test<T>(0)) == sizeof(char) }; \
	}; \
	template<typename T> \
	static constexpr bool has_##member##_v = has_##member<T>::value

/// @brief Contains all colt provided utilities
namespace colt {	
	
  /// @brief Contains size types
  namespace sizes
  {

    /*********************************
    * BYTE SIZES AND LITERALS
    *********************************/

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

	/*********************************
	* RANGES TYPES FOR SPLICING
	*********************************/

	/// @brief Contains type traits and tag types
	namespace traits
	{
		/// @brief Tag structure for a Range with no beginning offset
		struct RangeBeginT { static constexpr size_t value = 0; };
		/// @brief Tag structure for a Range with no end offset
		struct RangeEndT { static constexpr size_t value = std::numeric_limits<size_t>::max(); };
	}

	/// @brief Tag object for a Range with no beginning offset
	constexpr const traits::RangeBeginT Begin;
	/// @brief Tag object for a Range with no end offset
	constexpr const traits::RangeEndT End;

	/// @brief Symbolizes a range used for splicing views.
	/// A Range contains 2 fields: the offset to the beginning of the view,
	/// and the offset to the end of the view.
	/// As we would like a range to be able to represent the whole view without
	/// having to store the exact size of the view, a special value of the end offset
	/// (RangeEndT::value) is used to represents that end.
	class Range
	{
		size_t begin = traits::RangeBeginT::value;
		size_t end = traits::RangeEndT::value;

	public:

		/// @brief Constructs a Range.
		/// Precondition: begin <= end
		/// @param begin The index to the beginning of the Range
		/// @param end The index to the end of the Range
		constexpr Range(size_t begin, size_t end) noexcept
			: begin(begin), end(end) { assert(begin <= end && "Invalid Range!"); }
		
		/// @brief Constructs a Range that represents an empty Range
		constexpr Range() noexcept
			: begin(0), end(0) {}

		/// @brief Constructs a Range that represents a whole Range.
		/// Same as Range(RangeBeginT).
		/// @param  RangeBeginT
		/// @param  RangeEndT
		constexpr Range(traits::RangeBeginT, traits::RangeEndT) noexcept {}
		
		/// @brief Constructs a Range that represents a whole Range.
		/// Same as Range(RangeBeginT, RangeEndT).
		/// @param  RangeBeginT
		constexpr Range(traits::RangeBeginT) noexcept {}

		/// @brief Constructs a Range from begin, till end of Range.
		/// Same as Range(size_t, RangeEndT).
		/// @param begin The index to the beginning of the Range
		constexpr Range(size_t begin) noexcept
			: begin(begin) {}

		/// @brief Constructs a Range from begin, till end of Range.
		/// Same as Range(size_t).
		/// @param begin The index to the beginning of the Range
		/// @param  RangeEndT
		constexpr Range(size_t begin, traits::RangeEndT) noexcept
			: begin(begin) {}

		/// @brief Constructs a Range from the beginning to 'end'
		/// @param  RangeBeginT
		/// @param end The end of the Range
		constexpr Range(traits::RangeBeginT, size_t end) noexcept
			: end(end) {}

		/// @brief Check if the Range represents an empty view
		/// @return True if the Range is empty
		constexpr bool isNone() const noexcept { return begin == end; }
		
		/// @brief Check if the Range represents the whole view
		/// @return True if the Range represents the whole view
		constexpr bool isAll() const noexcept { return begin == traits::RangeBeginT::value && end == traits::RangeEndT::value; }

		/// @brief Get the size of the Range
		/// @return Size of the Range
		constexpr size_t getSize() const noexcept { return end - begin; }

		/// @brief Get the offset to the beginning of the range
		/// @return The offset to the beginning
		constexpr size_t getBeginOffset() const noexcept { return begin; }

		/// @brief Get the offset to the beginning of the range
		/// @return The offset to the end or RangeEndT::value for end of view
		constexpr size_t getEndOffset() const noexcept { return end; }

		/// @brief Returns an empty Range.
		/// Same as Range{}.
		/// @return Empty Range
		constexpr static Range getEmptyRange() noexcept { return Range{}; }
		/// @brief Returns a Range over the whole view.
		/// Same as Range{ Begin, End }.
		/// @return Whole Range
		constexpr static Range getWholeRange() noexcept { return Range{ Begin, End }; }
	};

	/*********************************
	* COMMON TRAITS AND HELPERS
	*********************************/	

  namespace traits
  {

    /********** HAS_MEMBER **********/

    COLT_HAS_MEMBER(getByteSize);
    COLT_HAS_MEMBER(getData);
    COLT_HAS_MEMBER(getSize);
    COLT_HAS_MEMBER(isEmpty);
    COLT_HAS_MEMBER(end);
    COLT_HAS_MEMBER(begin);

    /********** ITERATORS **********/

    template<typename T>
    /// @brief Check if a type provides a 'begin' and 'end' method
    /// @tparam T The type to check
    struct is_iterable
    {
      static constexpr bool value = has_begin_v<T> && has_end_v<T>;
    };

    template<typename T>
    /// @brief Short hand for is_iterable<T>::value
    /// @tparam T The type to check
    static constexpr bool is_iterable_v = is_iterable<T>::value;

    /********** BY VALUE **********/

    template<typename T>
    /// @brief Contains type field, which is T for trivial types, and const T& for non-trivial types
    /// @tparam T The type to copy
    struct copy_if_trivial { using type = typename std::conditional_t<std::is_trivial_v<T>, T, const T&>; };

    template<typename T>
    /// @brief Short hand for copy_if_trivial::type
    /// @tparam T The type to copy
    using copy_if_trivial_t = typename copy_if_trivial<T>::type;

    /********** TAGS **********/

    /// @brief Tag for constructing in place
    struct InPlaceT {};    

    /// @brief Tag for empty Optional
    struct NoneT {};

    /// @brief Represents O(1)
    struct ConstantComplexityT {};
    /// @brief Represents amortized O(1)
    struct AmortizedConstantComplexityT {};
    /// @brief Represents O(log(n))
    struct LogarithmicComplexityT {};
    /// @brief Represents O(n)
    struct LinearComplexityT {};
    /// @brief Represents O(n^2)
    struct QuadraticComplexityT {};

    template<typename T>
    /// @brief Check if a type is a tag (type used to differentiate overloads)
    /// @tparam T The type to check
    struct is_tag { static constexpr bool value = false; };

    template<>
    /// @brief InPlaceT is a tag
    struct is_tag<InPlaceT> { static constexpr bool value = true; };

    template<>
    /// @brief RangeBeginT is a tag
    struct is_tag<RangeBeginT> { static constexpr bool value = true; };

    template<>
    /// @brief RangeEndT is a tag
    struct is_tag<RangeEndT> { static constexpr bool value = true; };

    template<>
    /// @brief NoneT is a tag
    struct is_tag<NoneT> { static constexpr bool value = true; };

    template<>
    /// @brief ConstantComplexityT is a tag
    struct is_tag<ConstantComplexityT> { static constexpr bool value = true; };
    template<>
    /// @brief AmortizedConstantComplexityT is a tag
    struct is_tag<AmortizedConstantComplexityT> { static constexpr bool value = true; };
    template<>
    /// @brief LogarithmicComplexityT is a tag
    struct is_tag<LogarithmicComplexityT> { static constexpr bool value = true; };
    template<>
    /// @brief LinearComplexityT is a tag
    struct is_tag<LinearComplexityT> { static constexpr bool value = true; };
    template<>
    /// @brief QuadraticComplexityT is a tag
    struct is_tag<QuadraticComplexityT> { static constexpr bool value = true; };

    template<typename T>
    /// @brief Short hand for is_tag<T>::value
    /// @tparam T The type to check
    static constexpr bool is_tag_v = is_tag<T>::value;
  }

	/// @brief Tag object for constructing in place
	constexpr const traits::InPlaceT InPlace;

	/// @brief Tag object for empty Optional
	constexpr const traits::NoneT None;
}

#endif //!HG_COLT_COMMON