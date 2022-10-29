#ifndef HG_COLT_VIEW
#define HG_COLT_VIEW

#include "details/common.h"
#include "Iterators.h"

namespace colt {
  
  template<typename T>
  /// @brief Represents a non-owning contiguous view over a range of objects.
  /// Should be passed by value.
  /// @tparam T The type of the objects
  class ContiguousView
  {
    static_assert(!traits::is_tag_v<T>, "Cannot use tag struct as typename!");

    /// @brief Pointer to the beginning of the view
    const T* begin_ptr;
    /// @brief Count of items in the view
    size_t size;

  public:

    /*********************************
    *         CONSTRUCTORS
    *********************************/

    /// @brief Constructs a view over the range [begin, begin + view_size).
    /// @param begin The beginning of the view
    /// @param view_size The size of the view
    constexpr ContiguousView(const T* begin, size_t view_size) noexcept
      : begin_ptr(begin), size(view_size) {}

    /// @brief Constructs a view over the range [begin, end).
    /// Precondition: end > begin.
    /// @param begin The beginning of the view
    /// @param end The end of the view
    constexpr ContiguousView(const T* begin, const T* end) noexcept;      

    /// @brief Default trivial copy-constructor.
    /// @param  ContiguousView to copy
    constexpr ContiguousView(const ContiguousView&) noexcept = default;
    /// @brief Default trivial move-constructor.
    /// @param  ContiguousView to move
    constexpr ContiguousView(ContiguousView&&) noexcept = default;
    /// @brief Default trivial copy-assignment operator.
    /// @param  ContiguousView to copy
    /// @return Self
    constexpr ContiguousView& operator=(ContiguousView&&) noexcept = default;
    /// @brief Default trivial move-assignment operator.
    /// @param  ContiguousView to move
    /// @return Self
    constexpr ContiguousView& operator=(const ContiguousView&) noexcept = default;

    /*********************************
    *           METHODS
    *********************************/

    /// @brief Returns an iterator to the beginning of the view.
    /// @return Iterator to the beginning of the view
    constexpr ContiguousIterator<const T> begin() const noexcept { return begin_ptr; }

    /// @brief Returns an iterator past the end of the view.
    /// @return Iterator to the end of the view
    constexpr ContiguousIterator<const T> end() const noexcept { return begin_ptr + size; }

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
    /// Precondition: !is_empty()
    /// @return The first item of the view
    constexpr traits::copy_if_trivial_t<const T&> get_front() const noexcept;

    /// @brief Get the back of the view.
    /// Precondition: !is_empty()
    /// @return The last item of the view
    constexpr traits::copy_if_trivial_t<const T&> get_back() const noexcept;

    /// @brief Shortens the view from the front by 1.
    /// Precondition: !is_empty()
    constexpr void pop_front() noexcept;

    /// @brief Shortens the view from the front by N.
    /// Precondition: N <= size
    /// @param N The number of objects to pop
    constexpr void pop_front_n(size_t N) noexcept;

    /// @brief Shortens the view from the back by 1.
    /// Precondition: !is_empty()
    constexpr void pop_back() noexcept;

    /// @brief Shortens the view from the back by N.
    /// Precondition: N <= size
    /// @param N The number of objects to pop
    constexpr void pop_back_n(size_t N) noexcept;

    /// @brief Returns the object at index 'index' of the view.
    /// Precondition: index < size
    /// @param index The index of the object
    /// @return The object at index 'index'
    constexpr traits::copy_if_trivial_t<const T&> operator[](size_t index) const noexcept;

    /// @brief Splices a view using a range
    /// @param range The range to use for splicing
    /// @return Spliced view
    constexpr ContiguousView<T> splice_range(Range range) const noexcept;

    /// @brief Checks if the view contains an object
    /// @param what The object to search for
    /// @return True if found
    constexpr bool contains(traits::copy_if_trivial_t<const T&> what) const noexcept
    {
      for (size_t i = 0; i < size; i++)
      {
        if (begin_ptr[i] == what)
          return true;
      }
      return false;
    }

    friend constexpr bool operator==(const ContiguousView& a, const ContiguousView& b) noexcept
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

    friend constexpr bool operator!=(const ContiguousView& a, const ContiguousView& b) noexcept
    {
      if (a.get_size() != b.get_size())
        return true;
      for (size_t i = 0; i < a.get_size(); i++)
      {
        if (a[i] != b[i])
          return true;
      }
      return false;
    }
  };
  
  template<typename T>
  constexpr ContiguousView<T>::ContiguousView(const T* begin, const T* end) noexcept
    : begin_ptr(begin), size(end - begin)
  {
    assert(begin <= end && "'end' should be greater than 'begin'!");
  }

  template<typename T>
  constexpr sizes::ByteSize ContiguousView<T>::get_byte_size() const noexcept
  {
    return { size * sizeof(T) };
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> ContiguousView<T>::get_front() const noexcept
  {
    assert(!is_empty() && "View was empty!");
    return begin_ptr[0];
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> ContiguousView<T>::get_back() const noexcept
  {
    assert(!is_empty() && "View was empty!");
    return begin_ptr[size - 1];
  }

  template<typename T>
  constexpr void ContiguousView<T>::pop_front() noexcept
  {
    assert(!is_empty() && "View was empty!");
    ++begin_ptr;
    --size;
  }

  template<typename T>
  constexpr void ContiguousView<T>::pop_front_n(size_t N) noexcept
  {
    assert(N <= size && "View does not contain enough items!");
    begin_ptr += N;
    size -= N;
  }

  template<typename T>
  constexpr void ContiguousView<T>::pop_back() noexcept
  {
    assert(!is_empty() && "View was empty!");
    --size;
  }

  template<typename T>
  constexpr void ContiguousView<T>::pop_back_n(size_t N) noexcept
  {
    assert(N <= size && "View does not contain enough items!");
    size -= N;
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> ContiguousView<T>::operator[](size_t index) const noexcept
  {
    assert(index < size && "Invalid index!");
    return begin_ptr[index];
  }

  template<typename T>
  constexpr ContiguousView<T> ContiguousView<T>::splice_range(Range range) const noexcept
  {
    size_t begin = range.get_begin_offset();
    assert(begin < size && "Invalid begin offset for Range!");
    size_t end = range.get_end_offset();
    end = (end > size ? size : end);    
    return { begin_ptr + begin, end - begin };
  }

  template<typename T>
  static std::size_t hash(const ContiguousView<T>& view) noexcept
  {
    static_assert(traits::is_hashable_v<T>,
      "Type of ContiguousView should be hashable!");

    auto size = view.get_size();
    size = size > 64 ? 64 : size;

    std::size_t seed = view.get_size();
    for (size_t i = 0; i < size; i++)
      seed ^= GetHash(view[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    
    return seed;
  }

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