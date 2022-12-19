#ifndef HG_COLT_REFLECTION
#define HG_COLT_REFLECTION

#include "../details/common.h"
#include "../utility/Typedefs.h"
#include "../data_structs/String.h"

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
  namespace details
  {
    template<size_t index, typename T, typename...Args>
    struct get_index_impl
    {
      using type = typename get_index_impl<index - 1, Args...>::type;
    };

    template<typename T, typename... Args>
    struct get_index_impl<0, T, Args...>
    {
      using type = T;
    };
    template<size_t index, typename T, typename...Args>
    /// @brief Gets the type at index 'index' from a pack.
    /// Asserts that the index is valid.
    /// @tparam T The current type
    /// @tparam ...Args The rest of the types
    struct get_index
    {
      static_assert(index < sizeof...(Args) + 1, "Invalid index for get<>!");
      /// @brief The type at index 'index'
      using type = typename get_index_impl<index, T, Args...>::type;
    };
  }

  template<typename T, typename... Args>
  /// @brief List of types containing at least one type
  /// @tparam T The first type of the list
  /// @tparam ...Args The rest of the types
  struct type_list
  {
    /// @brief Size of the list (always greater than 0)
    static constexpr size_t size = sizeof...(Args) + 1;

    template<size_t index>
    /// @brief Gets the type at index 'index'
    using get = typename details::get_index<index, T, Args...>::type;
    /// @brief Same list with all its types made const
    using all_const = type_list<std::add_const_t<T>, std::add_const_t<Args>...>;
  };
   
  namespace details
  {
    template<typename T, typename T2, typename... Args>
    /// @brief Pops the first typename of a parameter pack
    /// @tparam T The first typename to pop
    /// @tparam T2 The second typename
    /// @tparam ...Args The reset of the typenames
    struct pop_first
    {
      /// @brief Resulting type of popping
      using result = type_list<T2, Args...>;
    };
  }

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
    /// @brief Name helper
    static constexpr StringView _1 = StringView{ "PTR<" };
    /// @brief Name helper
    static constexpr StringView _2 = StringView{ ">" };
  public:
    /// @brief Name of the type
    static constexpr StringView name = traits::join_v<_1, info<std::remove_reference_t<decltype(*std::declval<T>())>>::name, _2>;
    /// @brief Accesses the pointed to type
    using ptr_to = info<std::remove_reference_t<decltype(*std::declval<T>())>>;
  };

  template<typename T>
  /// @brief Overload responsible of adding '&' for reference types
  /// @tparam T The type on which to apply the transformation
  struct info<T, std::enable_if_t<std::is_lvalue_reference_v<T> && !std::is_const_v<T>>>
    : public class_info<T>
  {
  private:
    /// @brief Name helper
    static constexpr StringView _1 = StringView{ "&" };

  public:
    /// @brief Name of the type
    static constexpr StringView name = traits::join_v<info<std::remove_reference_t<T>>::name, _1>;
    /// @brief Accesses the referenced type
    using ref_to = info<std::remove_reference_t<T>>;
  };

  template<typename T>
  /// @brief Overload responsible of adding '&' for reference types
  /// @tparam T The type on which to apply the transformation
  struct info<T, std::enable_if_t<std::is_rvalue_reference_v<T> && !std::is_const_v<T>>>
    : public class_info<T>
  {
  private:
    /// @brief Name helper
    static constexpr StringView _1 = StringView{ "&&" };

  public:
    /// @brief Name of the type
    static constexpr StringView name = traits::join_v<info<std::remove_reference_t<T>>::name, _1>;
    /// @brief Accesses the referenced type
    using ref_to = info<std::remove_reference_t<T>>;
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
    /// @brief Name of the type
    static constexpr StringView name = traits::join_v<_1, info<std::decay_t<decltype(std::declval<T>())>>::name>;
    
    using members_type = typename
      info<std::decay_t<T>>::members_type::all_const;
  };  

  struct members_t {};
  inline constexpr members_t members;

  template <typename F, typename... Ts, typename =
    std::enable_if_t<std::conjunction_v<std::is_invocable<F, Ts>...>>>
  /// @brief Calls 'f' with each arguments
  /// @tparam F The lambda type
  /// @tparam ...Ts The parameter pack
  /// @tparam  SFINAE helper
  /// @param f The lambda to call
  /// @param ...args The arguments to pass to the lambda
  constexpr void for_each(F&& f, Ts&&... args) noexcept
  {
    (f(std::forward<Ts>(args)), ...);
  }

  template<typename F, typename T>
  /// @brief Calls 'f' with each registered member of 'of'
  /// @tparam F The lambda type
  /// @tparam T The object type
  /// @param  Helper type (usually 'members')
  /// @param of The object whose members to access
  /// @param f The lambda function
  constexpr void for_each(members_t, T&& of, F&& f) noexcept
  {
    info<std::decay_t<T>>::apply_for_members(std::forward<T>(of), std::forward<F>(f));
  }
}

#ifdef COLT_USE_IOSTREAMS

template<typename T, typename = std::enable_if_t<colt::refl::info<T>::exist() && !colt::traits::is_coutable_v<T>>>
static std::ostream& operator<<(std::ostream& os, const T& obj) noexcept
{
  os << "{\n";
  for_each(members, obj,
    [&os, i = 0ULL](auto&& a) mutable
    {
      os << "   " << info<std::decay_t<T>>::members_table[i++] << " ("
        << info<std::decay_t<decltype(a)>>::name << "): " << std::forward<decltype(a)>(a)
        << '\n';
    }
  );
  os << "}\n";
  return os;
}

#endif

#define DECLARE_BUILTIN(type) \
template<> \
struct colt::refl::info<std::decay_t<type>, void> : public colt::refl::class_info<std::decay_t<type>> {\
static constexpr StringView name = #type; \
using members_type = type_list<std::decay_t<type>>;\
template<typename On, typename F, typename = std::enable_if_t<std::is_same_v<std::decay_t<On>, std::decay_t<type>>>>\
static constexpr void apply_for_members(On&& obj, F&& fn) {\
    fn(obj);\
}\
}

namespace colt
{
  DECLARE_BUILTIN(bool);
  DECLARE_BUILTIN(char);
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

#define MEMBER_TYPE(name) , decltype(name)
#define MEMBER_NAMES(name) #name,
#define MEMBER_APPLY_FN(name) fn(obj.name);

//IDEAS: might use sizeof name to add to member names

#define DECLARE_TYPE(type, members) \
template<> \
struct colt::refl::info<std::decay_t<type>, void> : public colt::refl::class_info<std::decay_t<type>> {\
static constexpr StringView name = #type;\
using members_type = typename details::pop_first<void members(MEMBER_TYPE)>::result;\
private:\
using str = const char*;\
static constexpr const char* member_names[] = {\
  members(MEMBER_NAMES)\
  };\
public:\
  static constexpr ContiguousView<const str> members_table = { member_names, sizeof(member_names) / sizeof(const str) };\
  template<typename On, typename F, typename = std::enable_if_t<std::is_same_v<std::decay_t<On>, std::decay_t<type>>>>\
  static constexpr void apply_for_members(On&& obj, F&& fn) {\
    members(MEMBER_APPLY_FN)\
  }\
  static constexpr colt::iter::ContiguousView<const str> to_member_str_iter() noexcept {\
    return { member_names, sizeof(member_names) / sizeof(const str) }; }\
}

#endif //!HG_COLT_REFLECTION