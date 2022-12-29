/** @file Hash.h
* Contains hashing utilities used throughout the library.
* Use GetHash to hash an object. To add hashing support for
* an object, overload `std::hash` or `colt::hash`.
* `colt::hash` can be overloaded in the same way as `std::hash`,
* using operator().
* If both a `std::hash` and `colt::hash` overload are found,
* the `colt::hash` will take priority.
* `colt::hash` has been overloaded for every fundamental type,
* with `const char*` being treated as a NUL-terminated string:
* hashing of `const char*` will read each character of the string.
* This means that two identical strings not residing on the same address
* will give the same hash.
*/

#ifndef HG_COLT_HASH
#define HG_COLT_HASH

#include <functional>
#include <limits>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "../details/common.h"
#include "../utility/Typedefs.h"

namespace colt
{
  template<typename T>
  /// @brief Non-overloaded hash struct
  /// @tparam T Non-overloaded type
  struct hash {};

  template<>
  /// @brief colt::hash overload for bool
  struct hash<bool>
  {
    /// @brief Hashing operator
    /// @param b The value to hash
    /// @return Hash
    constexpr size_t operator()(bool b) const noexcept
    {
      return b ? 1231 : 1237;
    }
  };

  template<>
  /// @brief colt::hash overload for uint32_t
  struct hash<uint32_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(uint32_t i) const noexcept
    {
      size_t x = i;
      x = ((x >> 16) ^ x) * 0x45d9f3b;
      x = ((x >> 16) ^ x) * 0x45d9f3b;
      x = (x >> 16) ^ x;
      return x;
    }
  };

  template<>
  /// @brief colt::hash overload for uint64_t
  struct hash<uint64_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(uint64_t i) const noexcept
    {
      size_t x = i;
      x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
      x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
      x = x ^ (x >> 31);
      return x;      
    }
  };

  template<>
  /// @brief colt::hash overload for int16_t
  struct hash<int16_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(int16_t i) const noexcept
    {
      const auto in = static_cast<uint64_t>(i);
      return colt::hash<uint64_t>{}(in);
    }
  };

  template<>
  /// @brief colt::hash overload for uint16_t
  struct hash<uint16_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(uint16_t i) const noexcept
    {
      const auto in = static_cast<uint64_t>(i);
      return hash<uint64_t>{}(in);
    }
  };

  template<>
  /// @brief colt::hash overload for int32_t
  struct hash<int32_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(int32_t i) const noexcept
    {
      auto x = static_cast<size_t>(i);
      x = ((x >> 16) ^ x) * 0x45d9f3b;
      x = ((x >> 16) ^ x) * 0x45d9f3b;
      x = (x >> 16) ^ x;
      return x;
    }
  };

  template<>
  /// @brief colt::hash overload for int64_t
  struct hash<int64_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(int64_t i) const noexcept
    {
      auto x = static_cast<size_t>(i);
      x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
      x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
      x = x ^ (x >> 31);
      return x;
    }
  };

  template<>
  /// @brief colt::hash overload for char
  struct hash<char>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(char i) const noexcept
    {
      const auto in = static_cast<uint64_t>(i);
      return hash<uint64_t>{}(in);
    }
  };

  template<>
  /// @brief colt::hash overload for uint8_t
  struct hash<uint8_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(uint8_t i) const noexcept
    {
      const auto in = static_cast<uint64_t>(i);
      return hash<uint64_t>{}(in);
    }
  };

  template<>
  /// @brief colt::hash overload for int8_t
  struct hash<int8_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(int8_t i) const noexcept
    {
      const auto in = static_cast<uint64_t>(i);
      return hash<uint64_t>{}(in);
    }
  };

  namespace details
  {
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
  /// @brief colt::hash overload for "const char*"
  struct hash<const char*>
  {
    /// @brief Hashing operator
    /// @param str The value to hash
    /// @return Hash
    size_t operator()(const char* str) const noexcept
    {
      auto size = std::strlen(str);

      uint64_t hash = 0xCBF29CE484222325;
      for (size_t i = 0; i < size; i++)
      {
        hash ^= (uint8_t)str[i];
        hash *= 0x100000001B3; //FNV prime
      }
      return hash;
    }
  };

  template<typename T>
  /// @brief colt::hash overload for pointer types
  struct hash<T*>
  {
    /// @brief Hashing operator
    /// @param ptr The value to hash
    /// @return Hash
    size_t operator()(T* ptr) const noexcept
    {
      auto x = reinterpret_cast<std::uintptr_t>(ptr);
      x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
      x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
      x = x ^ (x >> 31);
      return x;
    }
  };

  template<>
  /// @brief colt::hash overload for float
  struct hash<float>
  {
    /// @brief Hashing operator
    /// @param flt The value to hash
    /// @return Hash
    size_t operator()(float flt) const noexcept
    {
      auto x = static_cast<size_t>(bit_cast<uint32_t>(flt));
      x = ((x >> 16) ^ x) * 0x45d9f3b;
      x = ((x >> 16) ^ x) * 0x45d9f3b;
      x = (x >> 16) ^ x;
      return x;
    }
  };

  template<>
  /// @brief colt::hash overload for double
  struct hash<double>
  {
    /// @brief Hashing operator
    /// @param dbl The value to hash
    /// @return Hash
    size_t operator()(double dbl) const noexcept
    {
      auto x = bit_cast<size_t>(dbl);
      x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
      x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
      x = x ^ (x >> 31);
      return x;
    }
  };

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
    struct is_colt_hashable<T, std::void_t<decltype(std::declval<colt::hash<T>>()(std::declval<T>()))>>
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
      return colt::hash<T>{}(obj);
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

  template<typename T1, typename T2>
  /// @brief colt::hash overload for std::pair
  struct hash<std::pair<T1, T2>>
  {
    /// @brief Hashing operator
    /// @param pair The value to hash
    /// @return Hash
    constexpr size_t operator()(const std::pair<T1, T2>& pair) const noexcept
    {
      size_t seed = 0;
      seed = HashCombine(seed, GetHash(pair.first));
      seed = HashCombine(seed, GetHash(pair.second));
      return seed;
    }
  };
}

#endif //!HG_COLT_HASH