#ifndef HG_COLT_ENUM
#define HG_COLT_ENUM

#include "Iterators.h"

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

#define ENUM_CASE_INDEX(name, assign) case assign: return name##_i;

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
  class EnumType {\
public:\
  enum : type { \
    ENUM_DEF(ENUM_NAME_S) \
  }; \
private:\
  static constexpr const char* array_str[] = {\
    ENUM_DEF(ENUM_STRING_S)\
  };\
  using str = const char*;\
public:\
  static constexpr size_t get_count() { return sizeof(array_str) / sizeof(const str); }\
  static constexpr size_t get_count() { return sizeof(array_str) / sizeof(const str); }\
  static constexpr size_t get_min() { return 0; }\
  static constexpr size_t get_max() { return count - 1; }\
  static constexpr const char* to_string(type dummy) noexcept {\
    return array_str[static_cast<size_t>(dummy)]; }\
  static constexpr colt::iter::Range to_value_iter() noexcept {\
    return { 0, sizeof(array_str) / sizeof(const str) }; }\
  static constexpr colt::iter::ContiguousView<const str> to_str_iter() noexcept {\
    return { array_str, sizeof(array_str) / sizeof(const str) }; }\
  }

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
  class EnumType {\
  enum : type { \
    ENUM_DEF(ENUM_NAME_N) \
  }; \
public:\
  enum : type { \
    ENUM_DEF(ENUM_NAME) \
  }; \
private:\
  static constexpr const char* array_str[] = {\
    ENUM_DEF(ENUM_STRING)\
  };\
  static constexpr type array_val[] = {\
    ENUM_DEF(ENUM_VALUE)\
  };\
  using str = const char*;\
  static constexpr type to_index(type dummy) noexcept {\
    switch (dummy) { \
      ENUM_DEF(ENUM_CASE_INDEX) \
      default: return -1; \
    } }\
public:\
  static constexpr size_t get_count() noexcept { return sizeof(array_str) / sizeof(const str); }\
  static constexpr size_t get_min() noexcept { return std::min({ ENUM_DEF(ENUM_VALUE) }); }\
  static constexpr size_t get_max() noexcept { return std::max({ ENUM_DEF(ENUM_VALUE) }); }\
  static constexpr const char* to_string(type dummy) noexcept {\
    return array_str[to_index(dummy)]; }\
  static constexpr colt::iter::ContiguousView<const type> to_value_iter() noexcept {\
    return { array_val, sizeof(array_val) / sizeof(const type) }; }\
  static constexpr colt::iter::ContiguousView<const str> to_str_iter() noexcept {\
    return { array_str, sizeof(array_str) / sizeof(const str) }; }\
  }

#endif