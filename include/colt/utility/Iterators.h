/** @file Iterators.h
* Contains iterator facilities used throughout the library.
* Colt iterators are iterators that support only one method: `next()`.
* This method must return an Optional containing the value or None if
* no more values can be produced.
* As C++ range-based for-loops do not support Colt iterators, an adapter
* (adapter_of<>) can be used to adapt any Colt iterators for use in for-loops.
*/

#ifndef HG_COLT_ITERATORS
#define HG_COLT_ITERATORS

#include "../details/common.h"
#include "../data_structs/Optional.h"

namespace colt
{
  template<typename T>
  /// @brief Contiguous Iterator, which is just a pointer
  /// @tparam T The type of the pointer
  using ContiguousIterator = T*;
}

namespace colt::traits
{
  template<typename T>
  /// @brief Trait for if a type provides a 'next()' method which returns an optional
  /// @tparam T The type to check for
  class is_colt_iter
  {
    template <typename C, typename = std::void_t<
      decltype(std::declval<C>().next().is_value())>
    >
    static bool test(decltype(&C::next));
    template <typename C>
    static void test(...);

  public:
    static constexpr bool value = std::is_same_v<decltype(test<T>(0)), bool>;
  };

  template<typename T>
  /// @brief Short-hand for 'is_colt_iter<T>::value'
  /// @tparam T The type to check for
  inline constexpr bool is_colt_iter_v = is_colt_iter<T>::value;

  template<typename T>
  /// @brief Check if a type provides a 'begin' and 'end' method
  /// @tparam T The type to check for
  struct is_std_iter
  {
    static constexpr bool value = has_begin_v<T> && has_end_v<T>;
  };

  template<typename T>
  /// @brief Short-hand for is_std_iter<T>::value
  /// @tparam T The type to check for
  inline constexpr bool is_std_iter_v = is_std_iter<T>::value;

  template<typename T>
  /// @brief Check if a type is iterable (colt/std)
  /// @tparam T The type to check for
  struct is_iter
  {
    static constexpr bool value = is_colt_iter_v<T> || is_std_iter_v<T>;
  };

  template<typename T>
  /// @brief Short-hand for is_iter<T>::value
  /// @tparam T 
  inline constexpr bool is_iter_v = is_iter<T>::value;

  template<typename T>
  /// @brief Return type of next().value() of T
  /// @tparam T The type whose next().value() type to deduce
  using colt_iter_item_t = decltype(std::declval<T>().next().get_value());
  template<typename T>
  /// /// @brief Return type of next().value() of T
  /// @tparam T The type whose next().value() type to deduce
  using colt_iter_value_t = decltype(std::declval<T>().next());
}

/// @brief Namespace containing iterators helper facilities
namespace colt::iter
{
  template<typename T>
  /// @brief Adapter for colt iterators to act as standard iterators
  /// @tparam T The iterator/generator
  struct adapter_of
  {
    //Type to adapt must be a Colt iterator.
    static_assert(traits::is_colt_iter_v<T>, "Expected a colt iterator!");

    /// @brief Type generated by the generator
    using item = std::decay_t<traits::colt_iter_item_t<T>>;

    /// @brief The generator to use
    T gen;

    template<typename... Args>
    /// @brief Constructs an adapter over T, forwarding the arguments
    ///        to the constructor of T
    /// @tparam ...Args The parameter pack
    /// @param ...args The argument pack to forward
    constexpr adapter_of(Args&&... args) noexcept
      : gen(std::forward<Args>(args)...) {}

    /// @brief Iterable type
    struct iter
    {
      /// @brief Pointer to generator or null when no value can be generated
      T* gen = nullptr;
      /// @brief Value generated
      item value = {};

      /// @brief Constructs an 'end()' iterator
      constexpr iter() noexcept = default;
      /// @brief Constructs an 'end()' iterator
      explicit constexpr iter(std::nullptr_t) noexcept {};

      /// @brief Constructs an iterator over 'ref' generator
      /// @param ref The generator from which to produce values
      explicit constexpr iter(T* ref) noexcept
        : gen(ref)
      {
        //Initialize the iterator, generating first value
        ++(*this);
      }

      /// @brief Obtains the value stored in the iterator
      /// @return Stored value
      constexpr item operator*() { return value; }

      /// @brief Advances the iterator, consuming the current value 
      ///        and generating a new one.
      /// When no value can be generated, 'gen' is assigned nullptr.
      /// @return The advanced iterator
      constexpr iter& operator++() noexcept
      {
        if (auto opt = gen->next())
          value = opt.get_value();
        else
          gen = nullptr;
        return *this;
      }

      /// @brief Check if this iterator is equal to another one
      /// @param b The iterator to compare against
      /// @return True if equal
      constexpr bool operator==(const iter& b) const noexcept { return gen == nullptr; }

      /// @brief Check if this iterator is not equal to another one
      /// @param b The iterator to compare against
      /// @return True if not equal
      constexpr bool operator!=(const iter& b) const noexcept { return gen != nullptr; }
    };

    /// @brief Returns an iterator to the first item
    /// @return Iterator to the first item or 'end()'
    constexpr iter begin() noexcept { return iter(&gen); }

    /// @brief Returns an invalid iterator that should not be dereferenced
    /// @return Iterator that should not be dereferenced
    constexpr iter end() noexcept { return iter{}; }
  };

  /// @brief Type system helper
  struct adapter_t {};

  /// @brief Value helper
  inline constexpr adapter_t adapt = {};

  template<typename T, typename = std::enable_if_t<traits::is_colt_iter_v<T>>>
  /// @brief Adapts a colt iterator to a valid range-for iterator
  /// @tparam T The colt iterator type
  /// @param iter The colt iterator to adapt
  /// @param a Value helper (usually 'adapt')
  /// @return The adapted iterator
  constexpr adapter_of<T> operator|(T&& iter, adapter_t a) noexcept
  {
    return std::forward<T>(iter);
  }

  /// @brief Colt iterator to generate the Fibonacci sequence
  class Fibonacci
  {
    /// @brief Current value
    size_t curr = 0;
    /// @brief Next value
    size_t next_a = 1;

  public:
    /// @brief Generates the next value in the Fibonacci sequence
    /// @return Next value in the Fibonacci sequence (never None)
    Optional<size_t> next() noexcept
    {
      size_t current = this->curr;

      this->curr = this->next_a;
      this->next_a = current + this->next_a;

      return current;
    }
  };

  /// @brief Generates number beginning from a number and incrementing.
  /// The name iota is taken from the programming language APL.
  template<typename IntT = size_t>
  class Iota
  {
    static_assert(std::is_integral_v<IntT>, "Iota must generate integers!");
    /// @brief The current number to increment
    IntT current;

  public:
    /// @brief Constructs an Iota sequence starting from 'begin'
    /// @param begin The number from which to begin counting
    constexpr Iota(IntT begin = 0) noexcept
      : current(begin) {}

    /// @brief Generates the next value in the Iota sequence
    /// @return Next value in the Iota sequence (never None)
    Optional<IntT> next() noexcept
    {
      return current++;
    }
  };  

  /// @brief Reverse IOTA, decrements a number.
  /// The name iota is taken from the programming language APL.
  template<typename IntT = size_t>
  class IotaR
  {
    static_assert(std::is_integral_v<IntT>, "IotaR must generate integers!");
    /// @brief The current number to decrement
    IntT current;

  public:
    /// @brief Constructs a reverse Iota sequence starting from 'begin'
    /// @param begin The number from which to begin counting backward
    constexpr IotaR(IntT begin) noexcept
      : current(begin) {}

    /// @brief Generates the next value in the Iota sequence
    /// @return Next value in the reverse Iota sequence (never None)
    Optional<IntT> next() noexcept
    {
      return current--;
    }
  };

  /// @brief Generate consecutive values ranging from [current, end)
  class Range
  {
    /// @brief The current value
    size_t current;
    /// @brief The end value
    size_t end;

  public:
    /// @brief Constructs a Range from [begin, end)
    /// @param begin The beginning of the range
    /// @param end The end of the range (non-inclusive)
    constexpr Range(size_t begin, size_t end) noexcept
      : current(begin), end(end) {}

    /// @brief Returns the next number in the sequence
    /// @return Generated number or None if 'end' was hit
    Optional<size_t> next() noexcept
    {
      if (current != end)
        return current++;
      return None;
    }
  };  

  /// @brief Range between [current, end), advancing by a 'step'
  class SteppedRange
  {
    /// @brief The current value
    size_t current;
    /// @brief The end value
    size_t end;
    /// @brief The step (by how much to increment)
    size_t step;

  public:
    /// @brief Constructs a Range from [begin, end)
    /// @param begin The beginning of the range
    /// @param end The end of the range (non-inclusive)
    /// @param step By how much to increment
    constexpr SteppedRange(size_t begin, size_t end, size_t step) noexcept
      : current(begin), end(end), step(step) {}

    /// @brief Returns the next number in the sequence
    /// @return Generated number or None if 'end' was hit
    Optional<size_t> next() noexcept
    {
      if (current < end)
      {
        size_t cpy = current;
        current += step;
        return cpy;
      }
      return None;
    }
  };

  template<typename T>
  /// @brief Iterator over a contiguous view of object
  /// @tparam T The object over which the iterator is spanning
  class ContiguousView
  {
    /// @brief Pointer to the item before the current one
    T* ptr;
    /// @brief Pointer to the item before the last one
    T* end;

  public:
    template<size_t sz>
    /// @brief Constructs a ContiguousView from an array
    /// @param array The array over which to create an iterator
    constexpr ContiguousView(const T (&array)[sz]) noexcept
      : ptr(array - 1), end(this->ptr + sz) {}

    /// @brief Constructs a ContiguousView from a pointer and a size
    /// @param ptr The pointer to the first object
    /// @param sz The count of object pointed to by 'ptr'
    constexpr ContiguousView(T* ptr, size_t sz) noexcept
      : ptr(ptr - 1), end(this->ptr + sz) {}

    /// @brief Returns the next value in the view
    /// @return Next value or None if no more value can be found
    Optional<T> next() noexcept
    {
      if (ptr == end)
        return None;
      ptr++;
      return *ptr;
    }
  };

  /// @brief Creates an iterable range between 'begin' and 'end' (non-inclusive).
  /// Similar to the python 'range' function.
  /// @param begin The beginning of the range
  /// @param end The end of the range (non-inclusive)
  /// @return Iterable over [begin, end)
  constexpr adapter_of<Range> range(size_t begin, size_t end) noexcept
  {
    return { begin, end };
  }

  /// @brief Creates an iterable range between 'begin' and 'end' (non-inclusive) advancing by 'step'.
  /// Similar to the python 'range' function.
  /// @param begin The beginning of the range
  /// @param end The end of the range (non-inclusive)
  /// @param step The step
  /// @return Iterable over [begin, end)
  constexpr adapter_of<SteppedRange> range(size_t begin, size_t end, size_t step) noexcept
  {
    return { begin, end, step };
  }

  /// @brief Type helper
  struct drop_iter
  {
    /// @brief Count of items to drop
    size_t by_how_many;
  };

  /// @brief Drops 'how_many' values from an iterator
  /// @param how_many How many values to skip
  /// @return Type representing the dropping
  constexpr drop_iter drop(size_t how_many) noexcept
  {
    return { how_many };
  }

  template<typename iter, typename = std::enable_if_t<traits::is_colt_iter_v<iter>>>
  /// @brief Drops 'drop' values from an iterator
  /// @tparam iter The colt iterator type
  /// @tparam  SFINAE helper
  /// @param iterator The iterator from which to drop
  /// @param drop How many values to drop
  /// @return Modified iterator
  constexpr auto operator|(iter&& iterator, drop_iter drop) noexcept
    -> iter
  {
    for (; drop.by_how_many != 0; --drop.by_how_many)
      drop.by_how_many *= static_cast<size_t>(iterator.next().is_value());
    return iterator;
  }

  /// @brief Type helper
  struct take_iter
  {
    /// @brief How many values to take at most
    size_t how_many;
  };

  /// @brief Take at most 'how_many' values from an iterator
  /// @param how_many The maximum taken values from the iterator
  /// @return 
  constexpr take_iter take(size_t how_many) noexcept
  {
    return { how_many };
  }

  template<typename iter>
  /// @brief Iterator that takes at most a number of values from another iterator
  /// @tparam iter The colt iterator type from which to extract values
  struct Take
  {
    static_assert(traits::is_colt_iter_v<iter>, "Expected a colt iterator!");

    /// @brief The iterator from which to extract values
    iter itera;
    /// @brief How many values can at most be taken
    size_t how_many;

    constexpr traits::colt_iter_value_t<iter> next() noexcept
    {
      if (how_many == 0)
        return None;
      --how_many;
      return itera.next();
    }
  };

  template<typename iter, typename = std::enable_if_t<traits::is_colt_iter_v<iter>>>
  /// @brief Take at most 'take' values from an iterator
  /// @tparam iter The colt iterator type
  /// @tparam  SFINAE helper
  /// @param iterator The iterator from which to take values
  /// @param drop How many values to at most take
  /// @return Modified iterator
  constexpr auto operator|(iter&& iterator, take_iter take) noexcept
    -> Take<iter>
  {
    return { iterator, take.how_many };
  }
}

#endif //!HG_COLT_ITERATORS