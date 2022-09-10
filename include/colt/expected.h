#ifndef HG_COLT_EXPECTED
#define HG_COLT_EXPECTED

#include "common.h"

namespace colt
{
  template<typename ExpectedTy, typename ErrorTy>
  class Expected
  {
    union
    {
      ExpectedTy expected;
      ErrorTy error;
    };

    bool is_error;

  public:
    constexpr Expected(traits::ErrorT) noexcept(std::is_default_constructible_v<ErrorTy>)
      : is_error(true)
    {
      new(&error) ErrorTy();
    }

    constexpr Expected(traits::ErrorT, traits::copy_if_trivial_t<const ErrorTy> value) noexcept(std::is_nothrow_copy_constructible_v<ErrorTy>)
      : is_error(true)
    {
      new(&error) ErrorTy(value);
    }

    template<typename T_ = ErrorTy, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    constexpr Expected(traits::ErrorT, ErrorTy&& to_move) noexcept(std::is_nothrow_move_constructible_v<ErrorTy>)
      : is_error(true)
    {
      new(&error) ErrorTy(std::move(to_move));
    }

    template<typename... Args>
    constexpr Expected(traits::InPlaceT, traits::ErrorT, Args&&... args) noexcept(std::is_nothrow_constructible_v<ErrorTy, Args...>)
      : is_error(true)
    {
      new(&error) ErrorTy(std::forward<Args>(args)...);
    }

    constexpr Expected() noexcept(std::is_default_constructible_v<ExpectedTy>)
      : is_error(false)
    {
      new(&expected) ExpectedTy();
    }

    constexpr Expected(traits::copy_if_trivial_t<const ExpectedTy> value) noexcept(std::is_nothrow_copy_constructible_v<ExpectedTy>)
      : is_error(false)
    {
      new(&expected) ExpectedTy(value);
    }

    template<typename T_ = ExpectedTy, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    constexpr Expected(ExpectedTy&& to_move) noexcept(std::is_nothrow_move_constructible_v<T>)
      : is_error(false)
    {
      new(&expected) ExpectedTy(std::move(to_move));
    }    

    template<typename... Args, typename = std::enable_if_t<!std::is_same_v<traits::get_first_t<Args...>, traits::ErrorT>>>
    constexpr Expected(traits::InPlaceT, Args&&... args) noexcept(std::is_nothrow_constructible_v<ExpectedTy, Args...>)
      : is_error(false)
    {
      new(&expected) ExpectedTy(std::forward<Args>(args)...);
    }

    constexpr Expected(const Expected& copy) noexcept(std::is_nothrow_copy_constructible_v<ExpectedTy>&& std::is_nothrow_copy_constructible_v<ErrorTy>)
      : is_error(copy.is_error)
    {
      if (is_error)
        new(&error) ErrorTy(copy.error);
      else
        new(&expected) ExpectedTy(copy.expected);
    }

    constexpr Expected(Expected&& move) noexcept(std::is_nothrow_move_constructible_v<ExpectedTy>&& std::is_nothrow_move_constructible_v<ErrorTy>)
      : is_error(move.is_error)
    {
      if (is_error)
        new(&error) ErrorTy(std::move(move.error));
      else
        new(&expected) ExpectedTy(std::move(move.expected));
    }

    ~Expected() noexcept(std::is_nothrow_destructible_v<ExpectedTy>&& std::is_nothrow_destructible_v<ErrorTy>)
    {
      if (is_error)
        error.~ErrorTy();
      else
        expected.~ExpectedTy();
    }

    constexpr bool isError() const noexcept { return is_error; }
    constexpr bool isExpected() const noexcept { return !is_error; }

    constexpr traits::copy_if_trivial_t<const ErrorTy> getError() const noexcept
    {
      assert(isError() && "'Expected' did not contain an error!");
      return error;
    }

    constexpr traits::copy_if_trivial_t<const ExpectedTy> getExpected() const noexcept
    {
      assert(isExpected() && "'Expected' did contained an error!");
      return expected;
    }

    constexpr traits::copy_if_trivial_t<const ExpectedTy> getExpectedOr(traits::copy_if_trivial_t<const ExpectedTy> or_value) const noexcept
    {
      if (isError())
        return or_value;
      return expected;
    }

    constexpr traits::copy_if_trivial_t<const ExpectedTy> getExpectedOrAbort() const noexcept
    {
      if (isError())
        std::abort();
      return expected;
    }

    constexpr traits::copy_if_trivial_t<const ExpectedTy> getExpectedOrAbort(void(*on_abort)(void) noexcept) const noexcept
    {
      assert(on_abort && "'on_abort' cannot be NULL!");
      if (isError())
      {
        on_abort();
        std::abort();
      }
      return expected;
    }
  };

#ifdef COLT_USE_IOSTREAMS
  template<typename ExpT, typename ErrT>
  std::ostream& operator<<(std::ostream& os, const Expected<ExpT, ErrT>& var)
  {
    if (var.isError())
      os << var.getError();
    else
      os << var.getExpected();
    return os;
  }
#endif
}

#endif //!HG_COLT_EXPECTED