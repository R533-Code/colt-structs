#ifndef HG_COLT_ENUM
#define HG_COLT_ENUM

#include "Iterators.h"
#include "View.h"
#include "Reflection.h"

namespace colt::iter
{
  /// @brief IOTA for enums with consecutive values
  template<typename T>
  class EnumRange
  {
    size_t current;
    size_t end;

  public:
    template<typename = std::enable_if_t<colt::refl::info<T>::is_enum()>>
    constexpr EnumRange() noexcept
      : current(colt::refl::info<T>::get_min()), end(colt::refl::info<T>::get_max() + 1) {}

    constexpr EnumRange(size_t begin, size_t end) noexcept
      : current(begin), end(end) {}

    Optional<T> next() noexcept
    {
      if (current != end)
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(current++));
      return None;
    }
  };
}

/// @brief Expansion macro for enum value definition
#define ENUM_NAME(name,assign) name = assign,
/// @brief
#define ENUM_NAME_N(name,assign) name##_i,
/// @brief Expansion macro for enum value definition with no assign value
#define ENUM_NAME_S(name) name,

/// @brief Expansion macro for enum to string conversion
#define ENUM_STRING(name,assign) #name,
/// @brief Expansion macro for enum to string conversion with no assign value
#define ENUM_STRING_S(name) #name,

// expansion macro for enum to string conversion
#define ENUM_CASE(name,assign) case name: return #name;

#define ENUM_CASE_INDEX(name, assign) case name: return name##_i;

/// @brief Expansion macro for enum value array
#define ENUM_VALUE(name,assign) assign,

/// @brief Declares a magic enum for which enums have assigned values.
/// Example:
/// ```c++
/// #define OS_ENUM(XX) \ 
///   XX(Windows) \ 
///   XX(Linux) \ 
///   XX(MacOs)
/// 
/// DECLARE_ENUM(OsEnum, uint8_t, OS_ENUM);
/// ```
#define DECLARE_ENUM(EnumType, type, ENUM_DEF) \
  enum class EnumType : type {\
    ENUM_DEF(ENUM_NAME_S) \
  }; \
  template<>\
  class colt::refl::info<EnumType, void> : public colt::refl::enum_info {\
public:\
  static constexpr StringView name = #EnumType;\
  enum : type { \
    ENUM_DEF(ENUM_NAME_S) \
  }; \
private:\
  static constexpr const char* array_str[] = {\
    ENUM_DEF(ENUM_STRING_S)\
  };\
  using str = const char*;\
public:\
  static constexpr bool is_consecutive_enum() noexcept { return true; }\
  static constexpr type to_index(EnumType dummy) noexcept {\
    return static_cast<type>(dummy); }\
  static constexpr size_t get_count() { return sizeof(array_str) / sizeof(const str); }\
  static constexpr size_t get_min() { return 0; }\
  static constexpr size_t get_max() { return get_count() - 1; }\
  static constexpr colt::iter::Range to_value_iter() noexcept {\
    return { 0, sizeof(array_str) / sizeof(const str) }; }\
  static constexpr colt::iter::EnumRange<EnumType> to_iter() noexcept {\
    return {};\
  }\
  static constexpr colt::iter::ContiguousView<const str> to_str_iter() noexcept {\
    return { array_str, sizeof(array_str) / sizeof(const str) }; }\
  static constexpr colt::ContiguousView<const str> str_table = { array_str, sizeof(array_str) / sizeof(const str) };\
  };\
  static constexpr const char* to_string(EnumType dummy) noexcept {\
    return colt::refl::info<EnumType>::str_table[static_cast<size_t>(dummy)]; }\

/// @brief Declares a magic enum for which enums have assigned values.
/// Example:
/// ```c++
/// #define OS_ENUM(XX) \ 
///   XX(Windows, 10) \ 
///   XX(Linux, 12) \ 
///   XX(MacOs, 50)
/// 
/// DECLARE_VALUE_ENUM(OsEnum, uint8_t, OS_ENUM);
/// ```
#define DECLARE_VALUE_ENUM(EnumType, type, ENUM_DEF) \
  enum class EnumType : type {\
    ENUM_DEF(ENUM_NAME) \
  }; \
  template<>\
  class colt::refl::info<EnumType> : public colt::refl::enum_info {\
  enum : type { \
    ENUM_DEF(ENUM_NAME_N) \
  }; \
public:\
  static constexpr StringView name = #EnumType;\
  enum : type { \
    ENUM_DEF(ENUM_NAME) \
  }; \
private:\
  static constexpr const char* array_str[] = {\
    ENUM_DEF(ENUM_STRING)\
  };\
  static constexpr EnumType array_val[] = {\
    ENUM_DEF((EnumType)ENUM_VALUE)\
  };\
  using str = const char*;\
public:\
  static constexpr bool is_consecutive_enum() noexcept { return false; }\
  static constexpr type to_index(EnumType dummy) noexcept {\
    switch (static_cast<type>(dummy)) { \
      ENUM_DEF(ENUM_CASE_INDEX) \
      default: return -1; \
    } }\
  static constexpr size_t get_count() noexcept { return sizeof(array_str) / sizeof(const str); }\
  static constexpr size_t get_min() noexcept { return std::min({ ENUM_DEF(ENUM_VALUE) }); }\
  static constexpr size_t get_max() noexcept { return std::max({ ENUM_DEF(ENUM_VALUE) }); }\
  static constexpr colt::iter::ContiguousView<const EnumType> to_iter() noexcept {\
    return { array_val, sizeof(array_val) / sizeof(const type) }; }\
  static constexpr colt::iter::ContiguousView<const type> to_value_iter() noexcept {\
    return { reinterpret_cast<const type* const>(array_val), sizeof(array_val) / sizeof(const type) }; }\
  static constexpr colt::iter::ContiguousView<const str> to_str_iter() noexcept {\
    return { array_str, sizeof(array_str) / sizeof(const str) }; }\
  static constexpr colt::ContiguousView<const str> str_table = { array_str, sizeof(array_str) / sizeof(const str) };\
  };\
  constexpr const char* to_string(EnumType dummy) noexcept {\
    return colt::refl::info<EnumType>::str_table[colt::refl::info<EnumType>::to_index(dummy)];}\

#endif