#ifndef HG_COLT_OPTIONAL
#define HG_COLT_OPTIONAL

#include "common.h"

namespace colt {

  template<typename T>
  /// @brief Manages an optionally contained value.
  /// @tparam T The optional type to hold
  class Optional
  {
    static_assert(!traits::is_tag_v<T>, "Cannot use tag struct as typename!");

    /// @brief Buffer for the optional object
    alignas(T) char opt_buffer[sizeof(T)];
    /// @brief True if no object is contained
    bool is_none;

  public:
    /// @brief Constructs an empty Optional.
    Optional() noexcept
      : is_none(true) {}
    
    /// @brief Constructs an empty Optional.
    /// Same as Optional().
    /// @param  NoneT: use None
    Optional(traits::NoneT) noexcept
      : is_none(true) {}

    /// @brief Copy constructs an object into the Optional.
    /// @param to_copy The object to copy
    Optional(traits::copy_if_trivial_t<const T> to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>)
      : is_none(false)
    {
      new(opt_buffer) T(to_copy);
    }

    /// @brief Move constructs an object into the Optional
    /// @param to_move The object to move
    template<typename T_ = T, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    Optional(T&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>)
      : is_none(false)
    {
      new(opt_buffer) T(std::move(to_move));
    }

    template<typename... Args>
    /// @brief Constructs an object into the Optional directly.
    /// @tparam ...Args The parameter pack
    /// @param  InPlaceT, use InPlace
    /// @param ...args The argument pack
    Optional(traits::InPlaceT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : is_none(false)
    {
      new(opt_buffer) T(std::forward<Args>(args)...);
    }
    
    /// @brief Copy constructor.
    /// @param to_copy The Optional to copy
    Optional(const Optional<T>& to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>)
      : is_none(to_copy.is_none)
    {
      if (!is_none)
        new(opt_buffer) T(*std::launder(reinterpret_cast<const T*>(to_copy.opt_buffer)));
    }

    /// @brief Move constructor.
    /// @param to_move The Optional to move
    Optional(Optional<T>&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>)
      : is_none(to_move.is_none)
    {
      if (!is_none)
        new(opt_buffer) T(std::move(*std::launder(reinterpret_cast<T*>(to_move.opt_buffer))));
    }

    /// @brief Destructor, destructs the value if it exist.
    ~Optional() noexcept(std::is_nothrow_destructible_v<T>)
    {
      reset();
    }

    /// @brief Check if the Optional contains a value.
    /// @return True if the Optional contains a value
    explicit operator bool() const noexcept { return !is_none; }

    /// @brief Check if the Optional contains a value.
    /// Same as !isNone().
    /// @return True if the Optional contains a value
    bool hasValue() const noexcept { return !is_none; }

    /// @brief Check if the Optional does not contain a value.
    /// Same as !hasValue().
    /// @return True if the Optional does not contain a value
    bool isNone() const noexcept { return is_none; }

    /// @brief Get the value contained in the Optional.
    /// Precondition: hasValue().
    /// @return The value contained
    traits::copy_if_trivial_t<T> getValue() const noexcept;

    /// @brief Move out the value contained in the Optional.
    /// Precondition: hasValue().
    /// @return The value contained in the Optional
    T&& stealValue() noexcept;

    /// @brief Destroy the stored value if it exists.
    /// Called automatically by the destructor.
    void reset() noexcept(std::is_nothrow_destructible_v<T>);
  };
  
  template<typename T>
  traits::copy_if_trivial_t<T> Optional<T>::getValue() const noexcept
  {
    assert(!is_none && "Optional does not contain a value!");
    traits::copy_if_trivial_t<T> to_ret = *std::launder(reinterpret_cast<const T*>(opt_buffer));
    return to_ret;
  }
  
  template<typename T>
  T&& Optional<T>::stealValue() noexcept
  {
    assert(!is_none && "Optional does not contain a value!");
    return std::move(*std::launder(reinterpret_cast<T*>(opt_buffer)));
  }
  
  template<typename T>
  inline void Optional<T>::reset() noexcept(std::is_nothrow_destructible_v<T>)
  {
    if (!is_none)
    {
      std::launder(reinterpret_cast<T*>(opt_buffer))->~T();
      is_none = true;
    }
  }

#ifdef COLT_USE_IOSTREAMS
  template<typename T>
  static std::ostream& operator<<(std::ostream& os, const Optional<T>& var)
  {
    static_assert(traits::is_coutable_v<T>, "Type of Optional should implement operator<<(std::ostream&)!");
    if (var.isNone())
      os << "None";
    else
      os << var.getValue();
    return os;
  }
#endif
}

#endif //!HG_COLT_OPTIONAL