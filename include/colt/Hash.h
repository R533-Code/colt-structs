#ifndef HG_COLT_HASH
#define HG_COLT_HASH

#include <functional>
#include <limits>
#include <cstdint>
#include <type_traits>

#include "details/common.h"

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
  std::size_t hash(const int16_t& i) noexcept
  {
    const auto in = static_cast<uint64_t>(i);
    return hash(in);
  }

  template<>
  std::size_t hash(const uint16_t& i) noexcept
  {
    const auto in = static_cast<uint64_t>(i);
    return hash(in);
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

  template<>
  std::size_t hash(const char& chr) noexcept
  {
    const auto i = static_cast<uint64_t>(chr);
    return hash(i);
  }

  template<>
  std::size_t hash(const uint8_t& i) noexcept
  {
    const auto in = static_cast<uint64_t>(i);
    return hash(in);
  }

  template<>
  std::size_t hash(const int8_t& i) noexcept
  {
    const auto in = static_cast<uint64_t>(i);
    return hash(in);
  }

  namespace details
  {
    using cstr = const char*;

    template<typename T>
    using ptr = T*;

    template<typename T>
    constexpr T xorshift(T n, int i) {
      return n ^ (n >> i);
    }

    constexpr uint32_t distribute(uint32_t n) {
      uint32_t p = 0x55555555UL; // pattern of alternating 0 and 1
      uint32_t c = 3423571495UL; // random uneven integer constant
      return c * xorshift(p * xorshift(n, 16), 16);
    }

    constexpr uint64_t distribute(uint64_t n) {
      uint64_t p = 0x5555555555555555ULL; // pattern of alternating 0 and 1
      uint64_t c = 17316035218449499591ULL;// random uneven integer constant
      return c * xorshift(p * xorshift(n, 32), 32);
    }

    template <typename T, typename S>
    constexpr std::enable_if_t<std::is_unsigned_v<T>, T> rotl(const T n, const S i)
    {
      const T m = (std::numeric_limits<T>::digits - 1);
      const T c = i & m;
      return (n << c) | (n >> ((T(0) - c) & m));
    }       
  }

  template<>
  std::size_t hash(const details::cstr& str) noexcept
  {
    auto size = std::strlen(str);
    size = size > 64 ? 64 : size;
    
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
    auto x = reinterpret_cast<std::uintptr_t>(ptr);
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
    /// @brief Check if a type implements a std::hash specialization
    /// @tparam T The type to check for
    /// @tparam  SFINAE helper
    struct is_std_hashable
    {
      static constexpr bool value = false;
    };

    template<typename T>
    /// @brief Check if a type implements a std::hash specialization
    /// @tparam T The type to check for
    /// @tparam  SFINAE helper
    struct is_std_hashable<T, std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>>
    {
      static constexpr bool value = true;
    };

    template <typename T>
    /// @brief Short hand for is_std_hashable<T>::value
    /// @tparam T The type to check for
    constexpr bool is_std_hashable_v = is_std_hashable<T>::value;

    template<typename T, typename = std::void_t<>>
    /// @brief Check if a type implements a colt::hash specialization
    /// @tparam T The type to check for
    /// @tparam  SFINAE helper
    struct is_colt_hashable
    {
      static constexpr bool value = false;
    };

    template<typename T>
    /// @brief Check if a type implements a colt::hash specialization
    /// @tparam T The type to check for
    /// @tparam  SFINAE helper
    struct is_colt_hashable<T, std::void_t<decltype(colt::hash(std::declval<T>()))>>
    {
      static constexpr bool value = true;
    };

    template <typename T>
    /// @brief Short hand for is_colt_hashable<T>::value
    /// @tparam T The type to check for
    constexpr bool is_colt_hashable_v = is_colt_hashable<T>::value;

    template<typename T>
    /// @brief Check if a type can be either hashed with colt::hash or std::hash
    /// @tparam T The type to check for
    struct is_hashable
    {
      static constexpr bool value = is_std_hashable_v<T> || is_colt_hashable_v<T>;
    };

    template <typename T>
    /// @brief Short hand for is_hashable<T>::value
    /// @tparam T The type to check for
    constexpr bool is_hashable_v = is_hashable<T>::value;
  }

  template<typename T>
  /// @brief Hashes an object with colt::hash if implemented, else using std::hash specialization
  /// @tparam T The type to hash
  /// @param obj The object to hash
  /// @return Hash
  inline std::size_t GetHash(const T& obj) noexcept
  {
    static_assert(traits::is_hashable_v<T>,
      "Type does not implement colt::hash or std::hash!");
    if constexpr (traits::is_colt_hashable_v<T>)
      return hash(obj);
    else
      return std::hash<T>{}(obj);
  }

  /// @brief Combines 2 hashes.
  /// Example Usage:
  /// ```c++
  /// size_t seed = 0;
  /// seed = HashCombine(seed, GetHash(...));
  /// seed = HashCombine(seed, GetHash(...));
  /// ```
  /// @param seed The first hash
  /// @param v The second hash
  /// @return The combined hash
  constexpr std::size_t HashCombine(std::size_t seed, std::size_t v)
  {
    return details::rotl(seed, std::numeric_limits<size_t>::digits / 3) ^ details::distribute(v);
  }
}

#endif //!HG_COLT_HASH