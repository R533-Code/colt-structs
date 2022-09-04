#ifndef HG_COLT_OPTIONAL
#define HG_COLT_OPTIONAL

#include "common.h"

namespace colt {

  template<typename T>
  class Optional
  {
    static_assert(!is_tag_v<T>, "Cannot use tag struct as typename!");

    alignas(T) char opt_buffer[sizeof(T)];
    bool is_none;

  public:
    Optional() noexcept
      : is_none(true) {}
    
    Optional(NoneT) noexcept
      : is_none(true) {}

    Optional(const std::enable_if_t<std::is_copy_constructible_v<T>, T>& to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>)
      : is_none(false)
    {
      new(opt_buffer) T(to_copy);
    }

    Optional(std::enable_if_t<std::is_move_constructible_v<T>, T>&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>)
      : is_none(false)
    {
      new(opt_buffer) T(std::move(to_move));
    }

    template<typename... Args>
    Optional(InPlaceT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : is_none(false)
    {
      new(opt_buffer) T(std::forward<Args>(args)...);
    }
    
    Optional(const std::enable_if_t<std::is_copy_constructible_v<T>, Optional<T>>& to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>)
      : is_none(to_copy.is_none)
    {
      if (!is_none)
        new(opt_buffer) T(reinterpret_cast<const T&>(to_copy.opt_buffer));
    }

    Optional(std::enable_if_t<std::is_move_constructible_v<T>, Optional<T>>&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>)
      : is_none(to_move.is_none)
    {
      if (!is_none)
        new(opt_buffer) T(std::move(reinterpret_cast<T&>(to_move.opt_buffer)));
    }

    ~Optional() noexcept(std::is_nothrow_destructible_v<T>)
    {
      reset();
    }

    /// @brief Check if the Optional contains a value
    /// @return True if the Optional contains a value
    operator bool() const noexcept { return !is_none; }

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
    copy_if_trivial_t<T> getValue() const noexcept;

    /// @brief Move out the value contained in the Optional.
    /// Precondition: hasValue().
    /// @return The value contained in the Optional
    T&& stealValue() noexcept;

    /// @brief Destroy the stored value if it exists.
    /// Called automatically by the destructor.
    void reset() noexcept(std::is_nothrow_destructible_v<T>);
  };
  
  template<typename T>
  copy_if_trivial_t<T> Optional<T>::getValue() const noexcept
  {
    assert(!is_none && "Optional does not contain a value!");
    copy_if_trivial_t<T> to_ret = reinterpret_cast<const T&>(opt_buffer);
    return to_ret;
  }
  
  template<typename T>
  T&& Optional<T>::stealValue() noexcept
  {
    assert(!is_none && "Optional does not contain a value!");
    return std::move(reinterpret_cast<T&>(opt_buffer));
  }
  
  template<typename T>
  inline void Optional<T>::reset() noexcept(std::is_nothrow_destructible_v<T>)
  {
    if (!is_none)
    {
      reinterpret_cast<T*>(opt_buffer)->~T();
      is_none = true;
    }
  }
}

#endif //!HG_COLT_OPTIONAL