/** @file Optional.h
* Contains the Optional class.
* Optional<T> can be either a value or None.
*/

#ifndef HG_COLT_OPTIONAL
#define HG_COLT_OPTIONAL

#include "../details/common.h"

namespace colt
{
  /// @brief Optional should contain a value
#define colt_optional_is_value this->is_value()

  template<typename T>
  /// @brief Manages an optionally contained value.
  /// @tparam T The optional type to hold
  class Optional
  {
    static_assert(!traits::is_tag_v<T>, "Cannot use tag struct as typename!");

    /// @brief Buffer for the optional object
    alignas(T) char opt_buffer[sizeof(T)];
    /// @brief True if no object is contained
    bool is_none_v;

  public:
    /// @brief Constructs an empty Optional.
    constexpr Optional() noexcept;      
    
    /// @brief Constructs an empty Optional.
    /// Same as Optional().
    /// @param  NoneT: use None
    constexpr Optional(traits::NoneT) noexcept;      

    /// @brief Copy constructs an object into the Optional.
    /// @param to_copy The object to copy
    constexpr Optional(traits::copy_if_trivial_t<const T&> to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>);      

    /// @brief Move constructs an object into the Optional
    /// @param to_move The object to move
    template<typename T_ = T, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    constexpr Optional(T&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>);      

    template<typename... Args>
    /// @brief Constructs an object into the Optional directly.
    /// @tparam ...Args The parameter pack
    /// @param  InPlaceT, use InPlace
    /// @param ...args The argument pack
    constexpr Optional(traits::InPlaceT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>);      
    
    /// @brief Copy constructor.
    /// @param to_copy The Optional to copy
    constexpr Optional(const Optional<T>& to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>);      

    /// @brief Move constructor.
    /// @param to_move The Optional to move
    constexpr Optional(Optional<T>&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>);      

    /// @brief Destructor, destructs the value if it exist.
    ~Optional()
      noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Check if the Optional contains a value.
    /// @return True if the Optional contains a value
    explicit constexpr operator bool() const noexcept { return !is_none_v; }

    /// @brief Check if the Optional contains a value.
    /// Same as !is_none().
    /// @return True if the Optional contains a value
    constexpr bool is_value() const noexcept { return !is_none_v; }

    /// @brief Check if the Optional does not contain a value.
    /// Same as !is_value().
    /// @return True if the Optional does not contain a value
    constexpr bool is_none() const noexcept { return is_none_v; }

    /// @brief Returns the stored value.
    /// @return The value
    /// @pre is_value() (colt_optional_is_value).
    constexpr const T* operator->() const noexcept;
    /// @brief Returns the stored value.
    /// @return The value
    /// @pre is_value() (colt_optional_is_value).
    constexpr T* operator->() noexcept;

    /// @brief Returns the stored value.
    /// @return The value.
    /// @pre is_value() (colt_optional_is_value).
    constexpr traits::copy_if_trivial_t<const T&> operator*() const& noexcept;
    /// @brief Returns the stored value.
    /// @return The value.
    /// @pre is_value() (colt_optional_is_value).
    constexpr T& operator*() & noexcept;
    /// @brief Returns the stored value.
    /// @return The value.
    /// @pre is_value() (colt_optional_is_value).
    constexpr traits::copy_if_trivial_t<const T&&> operator*() const&& noexcept;
    /// @brief Returns the stored value.
    /// @return The value.
    /// @pre is_value() (colt_optional_is_value).
    constexpr T&& operator*() && noexcept;

    /// @brief Returns the stored value.
    /// @return The value.
    /// @pre is_value() (colt_optional_is_value).
    constexpr traits::copy_if_trivial_t<const T&> get_value() const& noexcept;
    /// @brief Returns the stored value.
    /// @return The value.
    /// @pre is_value() (colt_optional_is_value).
    constexpr T& get_value() & noexcept;
    /// @brief Returns the stored value.
    /// @return The value.
    /// @pre is_value() (colt_optional_is_value).
    constexpr traits::copy_if_trivial_t<const T&&> get_value() const&& noexcept;
    /// @brief Returns the stored value.
    /// @return The value.
    /// @pre is_value() (colt_optional_is_value).
    constexpr T&& get_value() && noexcept;

    /// @brief Returns the value if contained, else 'default_value'
    /// @param default_value The value to return if the Optional is None
    /// @return The value or 'default_value'
    constexpr T get_value_or(T&& default_value) const&;
    /// @brief Returns the value if contained, else 'default_value'
    /// @param default_value The value to return if the Optional is None
    /// @return The value or 'default_value'
    constexpr T get_value_or(T&& default_value) &&;

    /// @brief Destroy the stored value if it exists, and sets the Optional to an empty one.
    /// Called automatically by the destructor.
    constexpr void reset()
      noexcept(std::is_nothrow_destructible_v<T>);
  };

  template<typename T>
  constexpr Optional<T>::Optional() noexcept
    : is_none_v(true) {}

  template<typename T>
  constexpr Optional<T>::Optional(traits::NoneT) noexcept
    : is_none_v(true) {}

  template<typename T>
  constexpr Optional<T>::Optional(traits::copy_if_trivial_t<const T&> to_copy)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
    : is_none_v(false)
  {
    new(opt_buffer) T(to_copy);
  }

  template<typename T>
  constexpr Optional<T>::Optional(const Optional<T>& to_copy)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
    : is_none_v(to_copy.is_none_v)
  {
    if (!is_none_v)
      new(opt_buffer) T(*std::launder(reinterpret_cast<const T*>(to_copy.opt_buffer)));
  }

  template<typename T>
  constexpr Optional<T>::Optional(Optional<T>&& to_move)
    noexcept(std::is_nothrow_move_constructible_v<T>)
    : is_none_v(to_move.is_none_v)
  {
    if (!is_none_v)
      new(opt_buffer) T(std::move(*std::launder(reinterpret_cast<T*>(to_move.opt_buffer))));
  }

  template<typename T>
  Optional<T>::~Optional()
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    reset();
  }

  template<typename T>
  constexpr const T* Optional<T>::operator->() const noexcept
  {
    CHECK_REQUIREMENT(colt_optional_is_value);
    return std::launder(reinterpret_cast<const T*>(opt_buffer));
  }
  
  template<typename T>
  constexpr T* Optional<T>::operator->() noexcept
  {
    CHECK_REQUIREMENT(colt_optional_is_value);
    return std::launder(reinterpret_cast<T*>(opt_buffer));
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> Optional<T>::operator*() const& noexcept
  {
    CHECK_REQUIREMENT(colt_optional_is_value);
    return *std::launder(reinterpret_cast<const T*>(opt_buffer));
  }
  
  template<typename T>
  constexpr T& Optional<T>::operator*() & noexcept
  {
    CHECK_REQUIREMENT(colt_optional_is_value);
    return *std::launder(reinterpret_cast<const T*>(opt_buffer));
  }
  
  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&&> Optional<T>::operator*() const&& noexcept
  {
    CHECK_REQUIREMENT(colt_optional_is_value);
    return *std::launder(reinterpret_cast<const T*>(opt_buffer));
  }
  
  template<typename T>
  constexpr T&& Optional<T>::operator*() && noexcept
  {
    CHECK_REQUIREMENT(colt_optional_is_value);
    return std::move(*std::launder(reinterpret_cast<T*>(opt_buffer)));
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> Optional<T>::get_value() const& noexcept
  {
    CHECK_REQUIREMENT(colt_optional_is_value);
    return *std::launder(reinterpret_cast<const T*>(opt_buffer));
  }

  template<typename T>
  constexpr T& Optional<T>::get_value() & noexcept
  {
    CHECK_REQUIREMENT(colt_optional_is_value);
    return *std::launder(reinterpret_cast<T*>(opt_buffer));
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&&> Optional<T>::get_value() const&& noexcept
  {
    CHECK_REQUIREMENT(colt_optional_is_value);
    return *std::launder(reinterpret_cast<const T*>(opt_buffer));
  }

  template<typename T>
  constexpr T&& Optional<T>::get_value() && noexcept
  {
    CHECK_REQUIREMENT(colt_optional_is_value);
    return std::move(*std::launder(reinterpret_cast<T*>(opt_buffer)));
  }

  template<typename T>
  constexpr T Optional<T>::get_value_or(T&& default_value) const&
  {
    return is_none_v ? static_cast<T>(std::forward<T>(default_value)) : **this;
  }
  
  template<typename T>
  constexpr T Optional<T>::get_value_or(T&& default_value) &&
  {
    return is_none_v ? static_cast<T>(std::forward<T>(default_value)) : std::move(**this);
  }

  template<typename T>
  constexpr void Optional<T>::reset()
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    if (!is_none_v)
    {
      reinterpret_cast<T*>(opt_buffer)->~T();
      is_none_v = true;
    }
  }

  template<typename T>
  template<typename T_, typename>
  constexpr Optional<T>::Optional(T&& to_move)
    noexcept(std::is_nothrow_move_constructible_v<T>)
    : is_none_v(false)
  {
    new(opt_buffer) T(std::move(to_move));
  }

  template<typename T>
  template<typename ...Args>
  constexpr Optional<T>::Optional(traits::InPlaceT, Args && ...args)
    noexcept(std::is_nothrow_constructible_v<T, Args ...>)
    : is_none_v(false)
  {
    new(opt_buffer) T(std::forward<Args>(args)...);
  }

  template<typename T>
  /// @brief Hash overload for ContiguousView
  /// @tparam T The type of the ContiguousView
  struct hash<Optional<T>>
  {
    /// @brief Hashing operator
    /// @param view The view to hash
    /// @return Hash
    constexpr size_t operator()(const Optional<T>& opt) const noexcept
    {
      static_assert(traits::is_hashable_v<T>,
        "Type of Optional should be hashable!");

      return opt.is_none() ? 18446744073709548283 : GetHash(opt.get_value());
    }
  };

#ifdef COLT_USE_IOSTREAMS
  template<typename T>
  static std::ostream& operator<<(std::ostream& os, const Optional<T>& var)
  {
    static_assert(traits::is_coutable_v<T>, "Type of Optional should implement operator<<(std::ostream&)!");
    if (var.is_none())
      os << "None";
    else
      os << var.get_value();
    return os;
  }
#endif
}

#endif //!HG_COLT_OPTIONAL