#ifndef HG_COLT_EXPECTED
#define HG_COLT_EXPECTED

#include "common.h"

namespace colt
{
  template<typename ExpectedTy, typename ErrorTy>
  /// @brief A helper class that can hold either a valid value or an error.
  /// This class can be seen as an Optional, that carries error informations.
  /// To document source code more, to constructs an Expected containing
  /// an error, colt::Error is passed to the constructor.
  /// The default constructor of an Expected default constructs an expected value:
  /// While this might seen counterintuitive, this class is named Expected, as it
  /// will most likely contain the expected value, not an error.
  ///
  /// Example Usage:
  /// ```C++
  /// Expected<int, const char*> div(int a, int b)
  /// {
  ///   if (b != 0)
  ///     return a / b;
  ///   return { Error, "Division by zero is prohibited!" };
  /// }
  /// ```
  /// @tparam ExpectedTy The expected type
  /// @tparam ErrorTy The error type
  class Expected
  {
    static_assert(!traits::is_tag_v<ErrorTy>, "Cannot use tag struct as typename!");
    static_assert(!traits::is_tag_v<ExpectedTy>, "Cannot use tag struct as typename!");

    /// @brief Buffer for both error type and expected value
    union
    {
      /// @brief The expected value (active when is_error == false)
      ExpectedTy expected;
      /// @brief The error value (active when is_error == true)
      ErrorTy error;
    };
    
    /// @brief True if an error is stored in the Expected
    bool is_error;

  public:
    /// @brief Default constructs an error in the Expected
    /// @param  ErrorT tag
    constexpr Expected(traits::ErrorT) noexcept(std::is_default_constructible_v<ErrorTy>);      

    /// @brief Copy constructs an error in the Expected
    /// @param  ErrorT tag
    /// @param value The value to copy
    constexpr Expected(traits::ErrorT, traits::copy_if_trivial_t<const ErrorTy> value) noexcept(std::is_nothrow_copy_constructible_v<ErrorTy>);

    template<typename T_ = ErrorTy, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    /// @brief Move constructs an error in the Expected
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param  ErrorT tag
    /// @param to_move The value to move
    constexpr Expected(traits::ErrorT, ErrorTy&& to_move) noexcept(std::is_nothrow_move_constructible_v<ErrorTy>);

    template<typename... Args>
    /// @brief Constructs an error in place in the Expected
    /// @tparam ...Args Parameter pack
    /// @param  InPlaceT tag
    /// @param  ErrorT tag
    /// @param ...args Argument pack forwarded to the constructor
    constexpr Expected(traits::InPlaceT, traits::ErrorT, Args&&... args) noexcept(std::is_nothrow_constructible_v<ErrorTy, Args...>);      

    /// @brief Default constructs an expected value in the Expected
    constexpr Expected() noexcept(std::is_default_constructible_v<ExpectedTy>);      

    /// @brief Copy constructs an expected value in the Expected
    /// @param value The value to copy
    constexpr Expected(traits::copy_if_trivial_t<const ExpectedTy> value) noexcept(std::is_nothrow_copy_constructible_v<ExpectedTy>);     

    template<typename T_ = ExpectedTy, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    /// @brief Move constructs an expected value in the Expected
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param to_move The value to move
    constexpr Expected(ExpectedTy&& to_move) noexcept(std::is_nothrow_move_constructible_v<T_>);    

    template<typename... Args, typename = std::enable_if_t<!std::is_same_v<traits::get_first_t<Args...>, traits::ErrorT>>>
    /// @brief Constructs an expected value in place in the Expected
    /// @tparam ...Args Parameter pack
    /// @param  InPlaceT tag
    /// @param  ErrorT tag
    /// @param ...args Argument pack forwarded to the constructor
    constexpr Expected(traits::InPlaceT, Args&&... args) noexcept(std::is_nothrow_constructible_v<ExpectedTy, Args...>);      

    /// @brief Copy constructs an Expected
    /// @param copy The Expected to copy
    constexpr Expected(const Expected& copy) noexcept(std::is_nothrow_copy_constructible_v<ExpectedTy> && std::is_nothrow_copy_constructible_v<ErrorTy>);

    /// @brief Copy assignment operator
    /// @param copy The Expected to copy
    /// @return Self
    constexpr Expected& operator=(const Expected& copy) noexcept(std::is_nothrow_copy_constructible_v<ExpectedTy> && std::is_nothrow_copy_constructible_v<ErrorTy>);

    /// @brief Move constructs an Expected
    /// @param move The Expected to move
    constexpr Expected(Expected&& move) noexcept(std::is_nothrow_move_constructible_v<ExpectedTy> && std::is_nothrow_move_constructible_v<ErrorTy>);

    /// @brief Move assignment operator
    /// @param move The Expected to move
    /// @return Self
    constexpr Expected& operator=(Expected&& move) noexcept(std::is_nothrow_move_constructible_v<ExpectedTy> && std::is_nothrow_move_constructible_v<ErrorTy>);
    
    /// @brief Destructs the value/error contained in the Expected
    ~Expected() noexcept(std::is_nothrow_destructible_v<ExpectedTy> && std::is_nothrow_destructible_v<ErrorTy>);

    /// @brief Check if the Expected contains an error
    /// @return True if the Expected contains an error
    constexpr bool isError() const noexcept { return is_error; }
    /// @brief Check if the Expected contains an expected value
    /// @return True if the Expected contains an expected value
    constexpr bool isExpected() const noexcept { return !is_error; }

    /// @brief Check if the Expected contains an error.
    /// Same as isError()
    /// @return True if the Expected contains an error
    constexpr bool operator!() const noexcept { return is_error; }
    /// @brief Check if the Expected contains an expected value.
    /// Same as isExpected()
    /// @return True if the Expected contains an expected value
    constexpr explicit operator bool() const noexcept { return !is_error; }

    /// @brief Returns the stored Expected value.
    /// Precondition: isExpected().
    /// @return The Expected value
    constexpr const ExpectedTy* operator->() const noexcept;
    /// @brief Returns the stored Expected value.
    /// Precondition: isExpected().
    /// @return The Expected value
    constexpr ExpectedTy* operator->() noexcept;    
    
    /// @brief Returns the stored Expected value.
    /// Precondition: isExpected()
    /// @return The Expected value.
    constexpr const ExpectedTy& operator*() const& noexcept;
    /// @brief Returns the stored Expected value.
    /// Precondition: isExpected().
    /// @return The Expected value.
    constexpr ExpectedTy& operator*() & noexcept;
    /// @brief Returns the stored Expected value.
    /// Precondition: isExpected()
    /// @return The Expected value.
    constexpr const ExpectedTy&& operator*() const&& noexcept;
    /// @brief Returns the stored Expected value.
    /// Precondition: isExpected()
    /// @return The Expected value.
    constexpr ExpectedTy&& operator*() && noexcept;    

    /// @brief Returns the Expected value if contained, else 'default_value'
    /// @param default_value The value to return if the Expected contains an error
    /// @return The Expected value or 'default_value'
    constexpr ExpectedTy getExpectedOr(ExpectedTy&& default_value) const&;
    /// @brief Returns the Expected value if contained, else 'default_value'
    /// @param default_value The value to return if the Expected contains an error
    /// @return The Expected value or 'default_value'
    constexpr ExpectedTy getExpectedOr(ExpectedTy&& default_value)&&;    

    /// @brief Returns the expected value, or aborts if it does not exist.
    /// @param on_abort The function to call before aborting or null
    /// @return The expected value
    constexpr const ExpectedTy& getExpectedOrAbort(void(*on_abort)(void) noexcept = nullptr) const& noexcept;
    /// @brief Returns the expected value, or aborts if it does not exist.
    /// @param on_abort The function to call before aborting or null
    /// @return The expected value
    constexpr ExpectedTy& getExpectedOrAbort(void(*on_abort)(void) noexcept = nullptr) & noexcept;
    /// @brief Returns the expected value, or aborts if it does not exist.
    /// @param on_abort The function to call before aborting or null
    /// @return The expected value
    constexpr const ExpectedTy&& getExpectedOrAbort(void(*on_abort)(void) noexcept = nullptr) const&& noexcept;
    /// @brief Returns the expected value, or aborts if it does not exist.
    /// @param on_abort The function to call before aborting or null
    /// @return The expected value
    constexpr ExpectedTy&& getExpectedOrAbort(void(*on_abort)(void) noexcept = nullptr) && noexcept;    
  };

  template<typename ExpectedTy, typename ErrorTy>
  constexpr Expected<ExpectedTy, ErrorTy>::Expected(traits::ErrorT) noexcept(std::is_default_constructible_v<ErrorTy>)
    : is_error(true)
  {
    new(&error) ErrorTy();
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr Expected<ExpectedTy, ErrorTy>::Expected(traits::ErrorT, traits::copy_if_trivial_t<const ErrorTy> value) noexcept(std::is_nothrow_copy_constructible_v<ErrorTy>)
    : is_error(true)
  {
    new(&error) ErrorTy(value);
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr Expected<ExpectedTy, ErrorTy>::Expected() noexcept(std::is_default_constructible_v<ExpectedTy>)
    : is_error(false)
  {
    new(&expected) ExpectedTy();
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr Expected<ExpectedTy, ErrorTy>::Expected(traits::copy_if_trivial_t<const ExpectedTy> value) noexcept(std::is_nothrow_copy_constructible_v<ExpectedTy>)
    : is_error(false)
  {
    new(&expected) ExpectedTy(value);
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr Expected<ExpectedTy, ErrorTy>::Expected(const Expected& copy) noexcept(std::is_nothrow_copy_constructible_v<ExpectedTy>&& std::is_nothrow_copy_constructible_v<ErrorTy>)
    : is_error(copy.is_error)
  {
    if (is_error)
      new(&error) ErrorTy(copy.error);
    else
      new(&expected) ExpectedTy(copy.expected);
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr Expected<ExpectedTy, ErrorTy>& Expected<ExpectedTy, ErrorTy>::operator=(const Expected& copy) noexcept(std::is_nothrow_copy_constructible_v<ExpectedTy>&& std::is_nothrow_copy_constructible_v<ErrorTy>)
  {
    assert(this != &copy && "Self assignment is prohibited!");

    if (is_error)
      error.~ErrorTy();
    else
      expected.~ExpectedTy();

    is_error = move.is_error;
    if (is_error)
      new(&error) ErrorTy(move.error);
    else
      new(&expected) ExpectedTy(move.expected);

    return *this;
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr Expected<ExpectedTy, ErrorTy>::Expected(Expected&& move) noexcept(std::is_nothrow_move_constructible_v<ExpectedTy>&& std::is_nothrow_move_constructible_v<ErrorTy>)
    : is_error(move.is_error)
  {
    if (is_error)
      new(&error) ErrorTy(std::move(move.error));
    else
      new(&expected) ExpectedTy(std::move(move.expected));
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr Expected<ExpectedTy, ErrorTy>& Expected<ExpectedTy, ErrorTy>::operator=(Expected&& move) noexcept(std::is_nothrow_move_constructible_v<ExpectedTy>&& std::is_nothrow_move_constructible_v<ErrorTy>)
  {
    assert(this != &copy && "Self assignment is prohibited!");

    if (is_error)
      error.~ErrorTy();
    else
      expected.~ExpectedTy();

    is_error = move.is_error;
    if (is_error)
      new(&error) ErrorTy(std::move(move.error));
    else
      new(&expected) ExpectedTy(std::move(move.expected));

    return *this;
  }

  template<typename ExpectedTy, typename ErrorTy>
  Expected<ExpectedTy, ErrorTy>::~Expected() noexcept(std::is_nothrow_destructible_v<ExpectedTy>&& std::is_nothrow_destructible_v<ErrorTy>)
  {
    if (is_error)
      error.~ErrorTy();
    else
      expected.~ExpectedTy();
  }

  template<typename ExpectedTy, typename ErrorTy>
  template<typename T_, typename>
  constexpr Expected<ExpectedTy, ErrorTy>::Expected(traits::ErrorT, ErrorTy&& to_move) noexcept(std::is_nothrow_move_constructible_v<ErrorTy>)
    : is_error(true)
  {
    new(&error) ErrorTy(std::move(to_move));
  }

  template<typename ExpectedTy, typename ErrorTy>
  template<typename ...Args>
  constexpr Expected<ExpectedTy, ErrorTy>::Expected(traits::InPlaceT, traits::ErrorT, Args && ...args) noexcept(std::is_nothrow_constructible_v<ErrorTy, Args ...>)
    : is_error(true)
  {
    new(&error) ErrorTy(std::forward<Args>(args)...);
  }

  template<typename ExpectedTy, typename ErrorTy>
  template<typename T_, typename>
  constexpr Expected<ExpectedTy, ErrorTy>::Expected(ExpectedTy&& to_move) noexcept(std::is_nothrow_move_constructible_v<T_>)
    : is_error(false)
  {
    new(&expected) ExpectedTy(std::move(to_move));
  }

  template<typename ExpectedTy, typename ErrorTy>
  template<typename ...Args, typename>
  constexpr Expected<ExpectedTy, ErrorTy>::Expected(traits::InPlaceT, Args && ...args) noexcept(std::is_nothrow_constructible_v<ExpectedTy, Args ...>)
    : is_error(false)
  {
    new(&expected) ExpectedTy(std::forward<Args>(args)...);
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr ExpectedTy& Expected<ExpectedTy, ErrorTy>::operator*() & noexcept
  {
    assert(!is_error && "Expected contained an error!");
    return expected;
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr const ExpectedTy* Expected<ExpectedTy, ErrorTy>::operator->() const noexcept
  {
    assert(!is_error && "Expected contained an error!");
    return &expected;
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr ExpectedTy* Expected<ExpectedTy, ErrorTy>::operator->() noexcept
  {
    assert(!is_error && "Expected contained an error!");
    return &expected;
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr const ExpectedTy& Expected<ExpectedTy, ErrorTy>::operator*() const& noexcept
  {
    assert(!is_error && "Expected contained an error!");
    return expected;
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr ExpectedTy&& Expected<ExpectedTy, ErrorTy>::operator*() && noexcept
  {
    assert(!is_error && "Expected contained an error!");
    return std::move(expected);
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr const ExpectedTy&& Expected<ExpectedTy, ErrorTy>::operator*() const&& noexcept
  {
    assert(!is_error && "Expected contained an error!");
    return expected;
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr ExpectedTy Expected<ExpectedTy, ErrorTy>::getExpectedOr(ExpectedTy&& default_value) const&
  {
    return is_error ? static_cast<ExpectedTy>(std::forward<ExpectedTy>(default_value)) : **this;
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr ExpectedTy Expected<ExpectedTy, ErrorTy>::getExpectedOr(ExpectedTy&& default_value)&&
  {
    return is_error ? static_cast<ExpectedTy>(std::forward<ExpectedTy>(default_value)) : std::move(**this);
  }

  template<typename ExpectedTy, typename ErrorTy>
  constexpr const ExpectedTy& Expected<ExpectedTy, ErrorTy>::getExpectedOrAbort(void(*on_abort)(void) noexcept) const& noexcept
  {
    if (is_error)
    {
      if (on_abort)
        on_abort();
      std::abort();
    }
    else
      return expected;
  }
  
  template<typename ExpectedTy, typename ErrorTy>
  constexpr ExpectedTy& Expected<ExpectedTy, ErrorTy>::getExpectedOrAbort(void(*on_abort)(void) noexcept) & noexcept
  {
    if (is_error)
    {
      if (on_abort)
        on_abort();
      std::abort();
    }
    else
      return expected;
  }
  
  template<typename ExpectedTy, typename ErrorTy>
  constexpr const ExpectedTy&& Expected<ExpectedTy, ErrorTy>::getExpectedOrAbort(void(*on_abort)(void) noexcept) const&& noexcept
  {
    if (is_error)
    {
      if (on_abort)
        on_abort();
      std::abort();
    }
    else
      return expected;
  }
  
  template<typename ExpectedTy, typename ErrorTy>
  constexpr ExpectedTy&& Expected<ExpectedTy, ErrorTy>::getExpectedOrAbort(void(*on_abort)(void) noexcept) && noexcept
  {
    if (is_error)
    {
      if (on_abort)
        on_abort();
      std::abort();
    }
    else
      return std::move(expected);
  }

#ifdef COLT_USE_IOSTREAMS
  template<typename ExpT, typename ErrT>
  std::ostream& operator<<(std::ostream& os, const Expected<ExpT, ErrT>& var)
  {
    static_assert(traits::is_coutable_v<ExpT> && traits::is_coutable_v<ErrT>,
      "Both types of Expected should implement operator<<(std::ostream&)!");

    if (var.isError())
      os << var.getError();
    else
      os << var.getExpected();
    return os;
  }
#endif
}

#endif //!HG_COLT_EXPECTED