#ifndef HG_COLT_COMMON
#define HG_COLT_COMMON

#include <cassert>
#include <cstring>

#include <mutex>
#include <atomic>

#include <type_traits>

#include <limits>
#include <memory>

#ifndef COLT_NO_DEBUG
  #if defined(NDEBUG) || defined(_DEBUG)
    /// @brief Defined if the library is compiled with assertions on.
    /// Allows parts of the code to be checked
    #define COLT_DEBUG
    /// @brief Does 'expr' only if COLT_DEBUG is defined
    #define COLT_ON_DEBUG(expr) do { expr; } while (0)
    /// @brief Expands to '&' only if COLT_DEBUG is defined
    #define COLT_REF_ON_DEBUG &
  #endif
#else
  /// @brief Expands to '&' only if COLT_DEBUG is defined
  #define COLT_REF_ON_DEBUG
  /// @brief Does 'expr' only if COLT_DEBUG is defined
  #define COLT_ON_DEBUG(expr) do { } while (0)
#endif

#ifdef COLT_USE_IOSTREAMS
  #include <iostream>
#endif

#ifdef COLT_USE_FMT
  #ifndef FMT_VERSION
    #undef COLT_USE_FMT
  #else
    #include <fmt/core.h>
  #endif
#endif

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

#ifdef COLT_USE_IOSTREAMS
    static std::ostream& operator<<(std::ostream& os, const ByteSize& var)
    {
      os << var.size << "B";
      return os;
    }
#endif

    /// @brief Represents a size in kilo-bytes
    struct KibiByteSize
    {
      /// @brief The kilo-byte size
      size_t size;

      /// @brief Conversion operator to ByteSize
      /// @return ByteSize
      constexpr operator ByteSize() const noexcept { return { size * 1024 }; }
    };

    /// @brief User defined literal to convert a value to a KibiByteSize
    /// @param size The size to convert
    /// @return KibiByteSize of size 'size'
    constexpr KibiByteSize operator"" _kiB(size_t size) noexcept { return { size }; }

#ifdef COLT_USE_IOSTREAMS
    static std::ostream& operator<<(std::ostream& os, const KibiByteSize& var)
    {
      os << var.size << "kiB";
      return os;
    }
#endif

    /// @brief Represents a size in mega-bytes
    struct MebiByteSize
    {
      /// @brief The mega-byte size
      size_t size;

      /// @brief Conversion operator to KibiByteSize
      /// @return KibiByteSize
      constexpr operator KibiByteSize() const noexcept { return { size * 1024 }; }
      /// @brief Conversion operator to ByteSize
      /// @return ByteSize
      constexpr operator ByteSize() const noexcept { return { size * 1024 * 1024 }; }
    };

    /// @brief User defined literal to convert a value to a MebiByteSize
    /// @param size The size to convert
    /// @return MebiByteSize of size 'size'
    constexpr MebiByteSize operator"" _MiB(size_t size) noexcept { return { size }; }

#ifdef COLT_USE_IOSTREAMS
    static std::ostream& operator<<(std::ostream& os, const MebiByteSize& var)
    {
      os << var.size << "MiB";
      return os;
    }
#endif

    /// @brief Represents a size in giga-bytes
    struct GibiByteSize
    {
      /// @brief The giga-byte size
      size_t size;

      /// @brief Conversion operator to MebiByteSize
      /// @return MebiByteSize
      constexpr operator MebiByteSize() const noexcept { return { size * 1024 }; }
      /// @brief Conversion operator to KibiByteSize
      /// @return KibiByteSize
      constexpr operator KibiByteSize() const noexcept { return { size * 1024 * 1024 }; }
      /// @brief Conversion operator to ByteSize
      /// @return ByteSize
      constexpr operator ByteSize() const noexcept { return { size * 1024 * 1024 * 1024 }; }
    };

    /// @brief User defined literal to convert a value to a GibiByteSize
    /// @param size The size to convert
    /// @return GibiByteSize of size 'size'
    constexpr GibiByteSize operator"" _GiB(size_t size) noexcept { return { size }; }

#ifdef COLT_USE_IOSTREAMS
    static std::ostream& operator<<(std::ostream& os, const GibiByteSize& var)
    {
      os << var.size << "GiB";
      return os;
    }
#endif
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
  constexpr inline const traits::RangeBeginT Begin;
  /// @brief Tag object for a Range with no end offset
  constexpr inline const traits::RangeEndT End;

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

#ifdef COLT_USE_IOSTREAMS
  static std::ostream& operator<<(std::ostream& os, const Range& var)
  {
    size_t begin = var.getBeginOffset();
    size_t end = var.getEndOffset();
    os << '[';
    if (begin == 0)
      os << "Begin, ";
    else
      os << begin << ", ";
    if (end == traits::RangeEndT::value)
      os << "End)";
    else if (end == 0)
      os << "Begin)";
    else
      os << end << ")";
    return os;
  }
#endif

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
    COLT_HAS_MEMBER(allocate);
    COLT_HAS_MEMBER(deallocate);
    COLT_HAS_MEMBER(owns);
    
    /********** PRINTABLE **********/

#ifdef COLT_USE_IOSTREAMS
    template<typename T, typename dummy = void>
    /// @brief Check if a type can be printed to std::cout through 'operator<<'
    /// @tparam T The type to check
    /// @tparam dummy SFINAE helper
    struct is_coutable
    {
      static constexpr bool value = false;
    };

    template<typename T>
    /// @brief Check if a type can be printed to std::cout through 'operator<<'
    /// @tparam T The type to check
    struct is_coutable<T, typename std::enable_if_t<std::is_same_v<decltype(std::cout << std::declval<T>()), std::ostream&>>>
    {
      static constexpr bool value = true;
    };

    template<typename T>
    /// @brief Short hand for is_coutable<T>::value
    /// @tparam T The type to check
    static constexpr bool is_coutable_v = is_coutable<T>::value;
#endif

    /********** MEMORY ALLOCATION **********/

    template<typename T>
    /// @brief Check if a type provides a 'allocate' and 'deallocate' method
    /// @tparam T The type to check
    struct is_allocator
    {
      static constexpr bool value = has_allocate_v<T> && has_deallocate_v<T>;
    };

    template<typename T>
    /// @brief Short hand for is_allocator<T>::value
    /// @tparam T The type to check
    static constexpr bool is_allocator_v = is_allocator<T>::value;

    template<typename T>
    /// @brief Check if a type provides a 'allocate', 'deallocate' and 'owns' method
    /// @tparam T The type to check
    struct is_owning_allocator
    {
      static constexpr bool value = is_allocator_v<T> && has_owns_v<T>;
    };

    template<typename T>
    /// @brief Short hand for is_owning_allocator<T>::value
    /// @tparam T The type to check
    static constexpr bool is_owning_allocator_v = is_owning_allocator<T>::value;

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
    /// @brief Contains type field, which is T for trivial types, and T for non-trivial types.
    /// Example: copy_if_trivial<const T&>::type operator[](size_t index) {}
    /// This would make the operator return a copy for trivial types.
    /// Do no use to return non-const references:
    /// Should be used with const references (const T&/const T&&).
    /// @tparam T The type to copy
    struct copy_if_trivial { using type = typename std::conditional_t<std::is_trivial_v<std::decay_t<T>>, std::decay_t<T>, T>; };

    template<typename T>
    /// @brief Short hand for copy_if_trivial::type.
    /// Example: copy_if_trivial<const T&>::type operator[](size_t index) {}
    /// This would make the operator return a copy for trivial types.
    /// Do no use to return non-const references:
    /// Should be used with const references (const T&/const T&&).
    /// @tparam T The type to copy
    using copy_if_trivial_t = typename copy_if_trivial<T>::type;

    /********** TAGS **********/

    /// @brief Tag for constructing in place
    struct InPlaceT {};    

    /// @brief Tag for empty Optional
    struct NoneT {};

    /// @brief Tag for error in Expected
    struct ErrorT {};

    /// @brief Tag for NUL terminator in StringView
    struct WithNULT {};

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
    /// @brief NoneT is a tag
    struct is_tag<ErrorT> { static constexpr bool value = true; };

    template<>
    /// @brief NoneT is a tag
    struct is_tag<WithNULT> { static constexpr bool value = true; };

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

    /********** PACKS **********/

    template<typename T, typename... Args>
    /// @brief Returns the first argument of a pack
    /// @tparam ...Args The type pack
    /// @tparam T The first argument
    struct get_first
    {
      using type = T;
    };

    template<typename T, typename... Args>
    /// @brief Short hand for get_first<T, Args...>::type
    /// @tparam ...Args The type pack
    /// @tparam T The first argument
    using get_first_t = typename get_first<T, Args...>::type;

    template<bool fi, bool... va>
    struct disjunction {};

    template<bool... va>
    struct disjunction<true, va...>
    {
      static constexpr bool value = true;
    };

    template<bool... va>
    struct disjunction<false, va...>
    {
      static constexpr bool value = disjunction<va...>::value;
    };

    template<bool fi>
    struct disjunction<fi>
    {
      static constexpr bool value = fi;
    };

    template<bool fi, bool... va>
    static constexpr bool disjunction_v = disjunction<fi, va...>::value;
  }

  /// @brief Tag object for constructing in place
  constexpr inline const traits::InPlaceT InPlace;

  /// @brief Tag object for empty Optional
  constexpr inline const traits::NoneT None;

  /// @brief Tag object for error in Expected
  constexpr inline const traits::ErrorT Error;

  /// @brief Tag object for NUL in StringView
  constexpr inline const traits::WithNULT WithNUL;

  /*********************************
  * FUNCTIONS HELPERS
  *********************************/

  template<typename T>
  constexpr T swap(T& o1, T& o2) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_assignable_v<T&, T>)
  {
    T old_value = std::move(o1);
    o1 = o2;
    o2 = old_value;
  }

  template<typename T, typename U = T>
  constexpr T exchange(T& obj, U&& new_value) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_assignable_v<T&, U>)
  {
    T old_value = std::move(obj);
    obj = std::forward<U>(new_value);
    return old_value;
  }

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
}

#endif //!HG_COLT_COMMON