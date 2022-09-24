#ifndef HG_COLT_HASH
#define HG_COLT_HASH

#include <functional>
#include <cstdint>
#include <type_traits>

#include "common.h"

namespace colt
{
  template<typename T>
  std::size_t hash(const T&) noexcept = delete;

  template<>
  std::size_t hash(const bool& b) noexcept
  {
    return b ? 1231 : 1237;
  }

  template<>
  std::size_t hash(const char& chr) noexcept
  {
    return chr;
  }

  template<>
  std::size_t hash(const uint8_t& i) noexcept
  {
    return i;
  }

  template<>
  std::size_t hash(const uint16_t& i) noexcept
  {
    return i;
  }

  template<>
  std::size_t hash(const uint32_t& i) noexcept
  {
    size_t x = i;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
  }

  template<>
  std::size_t hash(const uint64_t& i) noexcept
  {
    size_t x = i;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    x = x ^ (x >> 31);
    return x;
  }

  template<>
  std::size_t hash(const int8_t& i) noexcept
  {
    return static_cast<size_t>(i);
  }

  template<>
  std::size_t hash(const int16_t& i) noexcept
  {
    return static_cast<size_t>(i);
  }

  template<>
  std::size_t hash(const int32_t& i) noexcept
  {
    auto x = static_cast<size_t>(i);
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
  }

  template<>
  std::size_t hash(const int64_t& i) noexcept
  {
    auto x = static_cast<size_t>(i);
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    x = x ^ (x >> 31);
    return x;
  }

  namespace details
  {
    using cstr = const char*;

    template<typename T>
    using ptr = T*;
  }

  template<>
  std::size_t hash(const details::cstr& str) noexcept
  {
    auto size = std::strlen(str);
    size = size > 16 ? 16 : size;
    
    uint64_t hash = 0xCBF29CE484222325;
    for (size_t i = 0; i < size; i++)
    {
      hash ^= (uint8_t)str[i];
      hash *= 0x100000001B3; //FNV prime
    }
    return hash;
  }

  template<typename PtrT>
  std::size_t hash(const details::ptr<PtrT>& ptr) noexcept
  {
    auto x = static_cast<std::uintptr_t>(ptr);
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    x = x ^ (x >> 31);
    return x;
  }

  template<>
  std::size_t hash(const float& flt) noexcept
  {
    auto x = static_cast<size_t>(bit_cast<uint32_t>(flt));
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
  }

  template<>
  std::size_t hash(const double& dbl) noexcept
  {
    auto x = bit_cast<size_t>(dbl);
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    x = x ^ (x >> 31);
    return x;
  }  

  namespace traits
  {
    template<typename T, typename = std::void_t<>>
    struct is_std_hashable
    {
      static constexpr bool value = false;
    };

    template<typename T>
    struct is_std_hashable<T, std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>>
    {
      static constexpr bool value = true;
    };

    template <typename T>
    constexpr bool is_std_hashable_v = is_std_hashable<T>::value;

    template<typename T, typename = std::void_t<>>
    struct is_colt_hashable
    {
      static constexpr bool value = false;
    };

    template<typename T>
    struct is_colt_hashable<T, std::void_t<decltype(colt::hash<T>(std::declval<T>()))>>
    {
      static constexpr bool value = true;
    };

    template <typename T>
    constexpr bool is_colt_hashable_v = is_colt_hashable<T>::value;

    template<typename T>
    struct is_hashable
    {
      static constexpr bool value = is_std_hashable_v<T> || is_colt_hashable_v<T>;
    };

    template <typename T>
    constexpr bool is_hashable_v = is_hashable<T>::value;
  }

  template<typename T>
  constexpr std::size_t get_hash(const T& obj) noexcept
  {
    static_assert(traits::is_hashable_v<T>,
      "Type does not implement colt::hash or std::hash!");
    if constexpr (traits::is_colt_hashable_v<T>)
      return hash(obj);
    else
      return std::hash<T>{}(obj);
  }
}

#endif //!HG_COLT_HASH