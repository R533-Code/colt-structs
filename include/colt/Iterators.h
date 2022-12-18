#ifndef HG_COLT_ITERATORS
#define HG_COLT_ITERATORS

#include "details/common.h"
#include "Optional.h"

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
    static_assert(traits::is_colt_iter_v<T>, "Expected a colt iterator!");

    /// @brief Type generated by the generator
    using item = std::decay_t<traits::colt_iter_item_t<T>>;

    /// @brief The generator to use
    T gen;

    /// @brief Constructs an adapter over 'ref'
    /// @param ref The generator to adapt
    template<typename... Args>
    constexpr adapter_of(Args&&... args) noexcept
      : gen(std::forward<Args>(args)...) {}

    /// @brief Iterable type
    struct iter
    {
      /// @brief Pointer to generator
      T* gen;
      /// @brief Value generated
      item value = {};

      /// @brief Constructs an 'end()' iterator
      constexpr iter() noexcept
        : gen(nullptr) {}
      /// @brief Constructs an 'end()' iterator
      constexpr iter(std::nullptr_t) noexcept
        : gen(nullptr) {}

      /// @brief Constructor
      /// @param ref 
      constexpr iter(T* ref) noexcept
        : gen(ref)
      {
        //Initialize the iterator
        ++(*this);
      }

      /// @brief Obtains the value stored in the iterator
      /// @return Stored value
      constexpr item operator*() { return value; }

      /// @brief Advances the iterator, consuming the current value
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

  class Fibonacci
  {
    size_t curr = 0;
    size_t next_a = 1;

  public:
    Optional<size_t> next() noexcept
    {
      size_t current = this->curr;

      this->curr = this->next_a;
      this->next_a = current + this->next_a;

      return current;
    }
  };

  /// @brief IOTA
  class Iota
  {
    size_t current;

  public:
    constexpr Iota(size_t begin = 0) noexcept
      : current(begin) {}

    Optional<size_t> next() noexcept
    {
      return current++;
    }
  };  

  /// @brief Reverse IOTA
  class IotaR
  {
    size_t current;

  public:
    constexpr IotaR(size_t begin) noexcept
      : current(begin) {}

    Optional<size_t> next() noexcept
    {
      return current--;
    }
  };

  /// @brief Range between [current, end)
  class Range
  {
    size_t current;
    size_t end;

  public:
    constexpr Range(size_t begin, size_t end) noexcept
      : current(begin), end(end) {}

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
    size_t current;
    size_t end;
    size_t step;

  public:
    constexpr SteppedRange(size_t begin, size_t end, size_t step) noexcept
      : current(begin), end(end), step(step) {}

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
    constexpr ContiguousView(T (array)[sz]) noexcept
      : ptr(array - 1), end(this->ptr + sz) {}

    /// @brief Constructs a ContiguousView from a pointer and a size
    /// @param ptr The pointer to the first object
    /// @param sz The count of object pointed to by 'ptr'
    constexpr ContiguousView(T* ptr, size_t sz) noexcept
      : ptr(ptr - 1), end(this->ptr + sz) {}

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