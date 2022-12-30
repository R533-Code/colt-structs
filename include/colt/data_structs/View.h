/** @file View.h
* Contains ContiguousView, a non-owning view over contiguous objects.
* ContiguousView are lightweight objects (pointer + size), and thus
* can be passed by values.
* None of the operations of ContiguousView modify the objects the view
* spans on.
* Note that `splice_range()` does not assert on invalid ranges, rather
* returns empty views.
*/

#ifndef HG_COLT_VIEW
#define HG_COLT_VIEW

#include "../details/common.h"
#include "../utility/Hash.h"
#include "../utility/Iterators.h"

namespace colt {
  
  /// @brief ContiguousView should at least contain an item
#define colt_view_is_not_empty this->is_not_empty()
  /// @brief ContigousView should contain at least N items
#define colt_view_contains_N_item N <= size
  /// @brief ContiguousView's beginning should be smaller than end
#define colt_view_begin_smaller_end begin <= end
  /// @brief 'index' was greater or equal to size of ContiguousView
#define colt_view_index_smaller_size index < size

  template<typename T>
  /// @brief Represents a non-owning contiguous view over a range of objects.
  /// Should be passed by value.
  /// @tparam T The type of the objects
  class ContiguousView
  {
    //Tags (like decltype(colt::Error)) should not be instantiated.
    static_assert(!traits::is_tag_v<T>, "Cannot use tag struct as typename!");

    /// @brief Pointer to the beginning of the view, can be null
    const T* begin_ptr;
    /// @brief Count of items in the view
    size_t size;

  public:

    /*********************************
    *         CONSTRUCTORS
    *********************************/

    constexpr ContiguousView(const ContiguousView&) noexcept = default;
    constexpr ContiguousView(ContiguousView&&) noexcept = default;
    constexpr ContiguousView& operator=(ContiguousView&&) noexcept = default;
    constexpr ContiguousView& operator=(const ContiguousView&) noexcept = default;

    /// @brief Constructs a view over the range [begin, begin + view_size).
    /// @param begin The beginning of the view
    /// @param view_size The size of the view
    constexpr ContiguousView(const T* begin, size_t view_size) noexcept
      : begin_ptr(begin), size(view_size) {}

    /// @brief Constructs a view over the range [begin, end).
    /// @param begin The beginning of the view
    /// @param end The end of the view
    /// @pre \p end >= \p begin (colt_view_begin_smaller_end)
    constexpr ContiguousView(const T* begin, const T* end) noexcept;    

    /*********************************
    *           METHODS
    *********************************/

    /// @brief Returns an iterator to the beginning of the view.
    /// @return Iterator to the beginning of the view
    constexpr ContiguousIterator<const T> begin() const noexcept { return begin_ptr; }

    /// @brief Returns an iterator past the end of the view.
    /// @return Iterator to the end of the view
    constexpr ContiguousIterator<const T> end() const noexcept { return begin_ptr + size; }

    /// @brief Returns a colt iterator over the view
    /// @return Iterator over the whole view
    constexpr iter::ContiguousView<const T> to_iter() const noexcept { return { begin_ptr, size }; }

    /// @brief Returns a pointer to the beginning of the view.
    /// @return Pointer to the beginning of the view
    constexpr const T* get_data() const noexcept { return begin_ptr; }

    /// @brief Returns the count of object the view spans on.
    /// @return The count of objects
    constexpr size_t get_size() const noexcept { return size; }

    /// @brief Returns the byte size the view spans on.
    /// @return The byte size
    constexpr sizes::ByteSize get_byte_size() const noexcept;

    /// @brief Check if the view is empty.
    /// @return True if the size of the view is 0
    constexpr bool is_empty() const noexcept { return size == 0; }

    /// @brief Check if the view is not empty.
    /// @return True if the size of the view is not 0
    constexpr bool is_not_empty() const noexcept { return size != 0; }

    /// @brief Get the front of the view.
    /// @return The first item of the view
    /// @pre is_not_empty() == true (colt_view_begin_smaller_end)
    /// Precondition: !is_empty() (colt_view_is_not_empty)
    constexpr traits::copy_if_trivial_t<const T&> get_front() const noexcept;

    /// @brief Get the back of the view.
    /// @pre is_not_empty() == true (colt_view_begin_smaller_end)
    /// @return The last item of the view
    constexpr traits::copy_if_trivial_t<const T&> get_back() const noexcept;

    /// @brief Shortens the view from the front by 1.
    /// @pre is_not_empty() == true (colt_view_begin_smaller_end)
    constexpr void pop_front() noexcept;

    /// @brief Shortens the view from the front by N.
    /// @param N The number of objects to pop
    /// @pre N <= get_size() (colt_view_contains_N_item)
    constexpr void pop_front_n(size_t N) noexcept;

    /// @brief Shortens the view from the back by 1.
    /// @pre is_not_empty() == true (colt_view_begin_smaller_end)
    constexpr void pop_back() noexcept;

    /// @brief Shortens the view from the back by N.
    /// @param N The number of objects to pop
    /// @pre N <= get_size() (colt_view_contains_N_item)
    constexpr void pop_back_n(size_t N) noexcept;

    /// @brief Returns the object at index 'index' of the view.
    /// @param index The index of the object
    /// @return The object at index 'index'
    /// @pre \p index < get_size() (colt_view_index_smaller_size)
    constexpr traits::copy_if_trivial_t<const T&> operator[](size_t index) const noexcept;

    /// @brief Splices a view using a range.
    /// This method does not assert or have precondition, but instead
    /// will return empty views on invalid input.
    /// @param range The range to use for splicing
    /// @return Spliced view
    constexpr ContiguousView<T> splice_range(Range range) const noexcept;

    /// @brief Checks if the view contains an object
    /// @param what The object to search for
    /// @return True if found
    constexpr bool contains(traits::copy_if_trivial_t<const T&> what) const noexcept;

    /// @brief Check if two StringViews are equal
    /// @param a The first StringView
    /// @param b The second StringView
    /// @return True if both StringView are over the same items
    friend constexpr bool operator==(const ContiguousView& a, const ContiguousView& b) noexcept;
    /// @brief Check if two StringViews are not equal
    /// @param a The first StringView
    /// @param b The second StringView
    /// @return True if both StringView are over the same items
    friend constexpr bool operator!=(const ContiguousView& a, const ContiguousView& b) noexcept;    
  };
  
  template<typename T>
  constexpr ContiguousView<T>::ContiguousView(const T* begin, const T* end) noexcept
    : begin_ptr(begin), size(end - begin)
  {
    CHECK_REQUIREMENT(colt_view_begin_smaller_end);
  }

  template<typename T>
  constexpr sizes::ByteSize ContiguousView<T>::get_byte_size() const noexcept
  {
    return { size * sizeof(T) };
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> ContiguousView<T>::get_front() const noexcept
  {
    CHECK_REQUIREMENT(colt_view_is_not_empty);
    return begin_ptr[0];
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> ContiguousView<T>::get_back() const noexcept
  {
    CHECK_REQUIREMENT(colt_view_is_not_empty);
    return begin_ptr[size - 1];
  }

  template<typename T>
  constexpr void ContiguousView<T>::pop_front() noexcept
  {
    CHECK_REQUIREMENT(colt_view_is_not_empty);
    ++begin_ptr;
    --size;
  }

  template<typename T>
  constexpr void ContiguousView<T>::pop_front_n(size_t N) noexcept
  {
    CHECK_REQUIREMENT(colt_view_contains_N_item);
    begin_ptr += N;
    size -= N;
  }

  template<typename T>
  constexpr void ContiguousView<T>::pop_back() noexcept
  {
    CHECK_REQUIREMENT(colt_view_is_not_empty);
    --size;
  }

  template<typename T>
  constexpr void ContiguousView<T>::pop_back_n(size_t N) noexcept
  {
    CHECK_REQUIREMENT(colt_view_contains_N_item);
    size -= N;
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> ContiguousView<T>::operator[](size_t index) const noexcept
  {
    CHECK_REQUIREMENT(colt_view_index_smaller_size);
    return begin_ptr[index];
  }

  template<typename T>
  constexpr ContiguousView<T> ContiguousView<T>::splice_range(Range range) const noexcept
  {
    size_t begin = range.get_begin_offset();
    if (begin >= size)
      return {}; //empty view
    size_t end = range.get_end_offset();
    end = (end > size ? size : end);    
    return { begin_ptr + begin, end - begin };
  }

  template<typename T>
  constexpr bool ContiguousView<T>::contains(traits::copy_if_trivial_t<const T&> what) const noexcept
  {
    for (size_t i = 0; i < size; i++)
    {
      if (begin_ptr[i] == what)
        return true;
    }
    return false;
  }

  template<typename T>
  constexpr bool operator==(const ContiguousView<T>& a, const ContiguousView<T>& b) noexcept
  {
    if (a.get_size() != b.get_size())
      return false;
    for (size_t i = 0; i < a.get_size(); i++)
    {
      if (a[i] != b[i])
        return false;
    }
    return true;
  }

  template<typename T>
  constexpr bool operator!=(const ContiguousView<T>& a, const ContiguousView<T>& b) noexcept
  {
    return !(a == b);
  }

  template<typename T>
  /// @brief Hash overload for ContiguousView
  /// @tparam T The type of the ContiguousView
  struct hash<ContiguousView<T>>
  {
    /// @brief Hashing operator
    /// @param view The view to hash
    /// @return Hash
    constexpr size_t operator()(const ContiguousView<T>& view) const noexcept
    {
      static_assert(traits::is_hashable_v<T>,
        "Type of ContiguousView should be hashable!");

      std::size_t seed = view.get_size();
      for (size_t i = 0; i < view.get_size(); i++)
        seed ^= GetHash(view[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

      return seed;
    }
  };

#ifdef COLT_USE_IOSTREAMS
  template<typename T>
  static std::ostream& operator<<(std::ostream& os, const ContiguousView<T>& var)
  {
    static_assert(traits::is_coutable_v<T>, "Type of ContiguousView should implement operator<<(std::ostream&)!");
    os << '[';
    if (!var.is_empty())
      os << var.get_front();
    for (size_t i = 1; i < var.get_size(); i++)
      os << ", " << var[i];
    os << ']';
    return os;
  }
#endif
}

#endif //!HG_COLT_VIEW