#ifndef HG_COLT_REFLECTION
#define HG_COLT_REFLECTION

#include <type_traits>
#include <utility>
#include <array>
#include "Typedefs.h"

namespace colt::traits
{
  template <StringView const&... Strs>
  /// @brief Concatenates StringView at compile time
  struct join
  {
    /// @brief Concatenate all the StringView and returns an array storing the result
    static constexpr auto impl() noexcept
    {
      constexpr std::size_t len = (Strs.get_size() + ... + 0);
      std::array<char, len + 1> arr{};
      auto append = [i = 0, &arr](auto const& s) mutable {
        for (auto c : s) arr[i++] = c;
      };
      (append(Strs), ...);
      arr[len] = 0;
      return arr;
    }
    /// @brief Array of characters representing concatenated string
    static constexpr auto arr = impl();
    /// @brief Concatenation result
    static constexpr StringView value{ arr.data(), arr.data() + arr.size() - 1 };
  };  

  template <StringView const&... Strs>
  /// @brief Short-hand for join<...>::value
  static constexpr auto join_v = join<Strs...>::value;

}

/// @brief Contains reflection utilities
namespace colt::refl
{
  template<typename, typename = void>
  /// @brief Result of reflection over types
  /// @tparam  Type reflected on
  /// @tparam  SFINAE helper
  struct info
  {
    /// @brief True if the type has reflected data
    /// @return false (non-specialized info)
    static constexpr bool exist() noexcept { return false; }    
    /// @brief True if the reflected data is describing an enum
    /// @return false (non-specialized info)
    static constexpr bool is_enum() noexcept { return false; }
    /// @brief True if the reflected data is describing a class
    /// @return false (non-specialized info)
    static constexpr bool is_class() noexcept { return false; }
    /// @brief True if the reflected data is describing a pointer
    /// @return false (non-specialized info)
    static constexpr bool is_pointer() noexcept { return false; }
    /// @brief True if the reflected data is describing a pointer
    /// @return false (non-specialized info)
    static constexpr bool is_ref() noexcept { return false; }
  };

  /// @brief Helper from which to inherit publicly for enums
  struct enum_info
  {
    /// @brief True (as specialized info should inherit this type)
    /// @return True
    static constexpr bool exist() noexcept { return true; }
    /// @brief True as enum
    /// @return True
    static constexpr bool is_enum() noexcept { return true; }
    /// @brief False as not class
    /// @return False
    static constexpr bool is_class() noexcept { return false; }
    /// @brief True if the reflected data is describing a pointer
    /// @return false (non-specialized info)
    static constexpr bool is_pointer() noexcept { return false; }
    /// @brief True if the reflected data is describing a pointer
    /// @return false (non-specialized info)
    static constexpr bool is_ref() noexcept { return false; }
  };

  template<typename T>
  /// @brief Helper from which to inherit publicly for classes
  struct class_info
  {
    /// @brief True (as specialized info should inherit this type)
    /// @return True
    static constexpr bool exist() noexcept { return true; }
    /// @brief False as not enum
    /// @return False
    static constexpr bool is_enum() noexcept { return false; }
    /// @brief True as class
    /// @return True
    static constexpr bool is_class() noexcept { return !std::is_pointer_v<T>; }
    /// @brief True if the reflected data is describing a pointer
    /// @return false (non-specialized info)
    static constexpr bool is_pointer() noexcept { return std::is_pointer_v<T>; }
    /// @brief True if the reflected data is describing a pointer
    /// @return false (non-specialized info)
    static constexpr bool is_ref() noexcept { return std::is_reference_v<T>; }
  };

  template<typename T>
  /// @brief Overload responsible of adding 'PTR<...>' for pointer types
  /// @tparam T The type on which to apply the transformation
  struct info<T, std::enable_if_t<std::is_pointer_v<T> && !std::is_const_v<T>>>
    : public class_info<T>
  {
  private:
    static constexpr StringView _1 = StringView{ "PTR<" };
    static constexpr StringView _2 = StringView{ ">" };
  public:
    static constexpr StringView name = traits::join_v<_1, info<std::remove_reference_t<decltype(*std::declval<T>())>>::name, _2>;
  };

  template<typename T>
  /// @brief Overload responsible of adding '&' for reference types
  /// @tparam T The type on which to apply the transformation
  struct info<T, std::enable_if_t<std::is_reference_v<T> && !std::is_const_v<T>>>
    : public class_info<T>
  {
  private:
    static constexpr StringView _1 = StringView{ "&" };

  public:
    static constexpr StringView name = traits::join_v<info<std::remove_reference_t<T>>::name, _1>;
  };

  template<typename T>
  /// @brief Overload responsible of adding 'const' to class name
  /// @tparam T The type on which to apply the transformation
  struct info<T, std::enable_if_t<std::is_const_v<T> && info<std::decay_t<T>>::exist()>>
    : public class_info<T>, public info<std::decay_t<T>>
  {
  private:
    static constexpr StringView _1 = StringView{ "const " };
  public:
    static constexpr StringView name = traits::join_v<_1, info<std::decay_t<decltype(std::declval<T>())>>::name>;
  };
}

#define DECLARE_BUILTIN(type) \
template<> \
struct colt::refl::info<std::decay_t<type>, void> : public colt::refl::class_info<std::decay_t<type>> {\
static constexpr StringView name = #type; \
}

namespace colt
{
  DECLARE_BUILTIN(i8);
  DECLARE_BUILTIN(u8);
  DECLARE_BUILTIN(i16);
  DECLARE_BUILTIN(u16);
  DECLARE_BUILTIN(i32);
  DECLARE_BUILTIN(u32);
  DECLARE_BUILTIN(i64);
  DECLARE_BUILTIN(u64);
  DECLARE_BUILTIN(f32);
  DECLARE_BUILTIN(f64);
}


#endif //!HG_COLT_REFLECTION