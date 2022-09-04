#ifndef HG_COLT_VIEW
#define HG_COLT_VIEW

#include "common.h"

namespace colt {
  
  template<typename T>
  /// @brief Represents a contiguous view over a range of objects
  /// @tparam T The type of the objects
  class ContiguousView
  {
    static_assert(!is_tag_v<T>, "Cannot use tag struct as typename!");

    /// @brief Pointer to the beginning of the view
    const T* begin_ptr;
    /// @brief Count of items in the view
    size_t size;

  public:
    /*********************************
    *         CONSTRUCTORS
    *********************************/

    /// @brief Constructs a view over the range [begin, begin + view_size)
    /// @param begin The beginning of the view
    /// @param view_size The size of the view
    constexpr ContiguousView(const T* begin, size_t view_size) noexcept
      : begin_ptr(begin), size(view_size) {}

    /// @brief Constructs a view over the range [begin, end).
    /// Precondition: end > begin.
    /// @param begin The beginning of the view
    /// @param end The end of the view
    constexpr ContiguousView(const T* begin, const T* end) noexcept;      

    /// @brief Default trivial copy-constructor
    /// @param  ContiguousView to copy
    constexpr ContiguousView(const ContiguousView&) noexcept = default;
    /// @brief Default trivial move-constructor
    /// @param  ContiguousView to move
    constexpr ContiguousView(ContiguousView&&) noexcept = default;
    /// @brief Default trivial copy-assignment operator
    /// @param  ContiguousView to copy
    /// @return Self
    constexpr ContiguousView& operator=(ContiguousView&&) noexcept = default;
    /// @brief Default trivial move-assignment operator
    /// @param  ContiguousView to move
    /// @return Self
    constexpr ContiguousView& operator=(const ContiguousView&) noexcept = default;

    /*********************************
    *           METHODS
    *********************************/

    /// @brief Returns an iterator to the beginning of the view
    /// @return Iterator to the beginning of the view
    constexpr const T* begin() const noexcept { return begin_ptr; }
    /// @brief Returns an iterator past the end of the view
    /// @return Iterator to the end of the view
    constexpr const T* end() const noexcept { return begin_ptr + size; }

    /// @brief Returns a pointer to the beginning of the view
    /// @return Pointer to the beginning of the view
    constexpr const T* getData() const noexcept { return begin_ptr; }

    /// @brief Returns the count of object the view spans on
    /// @return The count of objects
    constexpr size_t getSize() const noexcept { return size; }    

    /// @brief Returns the byte size the view spans on
    /// @return The byte size
    constexpr ByteSize getByteSize() const noexcept;

    /// @brief Check if the view is empty
    /// @return True if the size of the view is 0
    constexpr bool isEmpty() const noexcept { return size == 0; }

    /// @brief Get the front of the view.
    /// Precondition: !isEmpty()
    /// @return The first item of the view
    constexpr copy_if_trivial<T> getFront() const noexcept;

    /// @brief Get the back of the view.
    /// Precondition: !isEmpty()
    /// @return The last item of the view
    constexpr copy_if_trivial<T> getBack() const noexcept;

    /// @brief Shortens the view from the front by 1.
    /// Precondition: !isEmpty()
    constexpr void popFront() const noexcept;

    /// @brief Shortens the view from the front by N.
    /// Precondition: N <= size
    /// @param N The number of objects to pop
    constexpr void popFrontN(size_t N) const noexcept;

    /// @brief Shortens the view from the back by 1.
    /// Precondition: !isEmpty()
    constexpr void popBack() const noexcept;

    /// @brief Shortens the view from the back by N.
    /// Precondition: N <= size
    /// @param N The number of objects to pop
    constexpr void popBackN(size_t N) const noexcept;

    /// @brief Returns the object at index 'index' of the view.
    /// Precondition: index < size
    /// @param index The index of the object
    /// @return The object at index 'index'
    constexpr copy_if_trivial_t<T> operator[](size_t index) const noexcept;

    /// @brief Splices a view using a range
    /// @param range The range to use for splicing
    /// @return Spliced view
    constexpr ContiguousView<T> spliceRange(Range range) const noexcept;  
  };
  
  template<typename T>
  constexpr ContiguousView<T>::ContiguousView(const T* begin, const T* end) noexcept
    : begin_ptr(begin), size(end - begin)
  {
    assert(begin < end && "'end' should be greater than 'begin'!");
  }

  template<typename T>
  constexpr ByteSize ContiguousView<T>::getByteSize() const noexcept
  {
    return ByteSize{ size * sizeof(T) };
  }

  template<typename T>
  constexpr copy_if_trivial<T> ContiguousView<T>::getFront() const noexcept
  {
    assert(!isEmpty() && "View was empty!");
    return begin_ptr[0];
  }

  template<typename T>
  constexpr copy_if_trivial<T> ContiguousView<T>::getBack() const noexcept
  {
    assert(!isEmpty() && "View was empty!");
    return begin_ptr[size - 1];
  }

  template<typename T>
  constexpr void ContiguousView<T>::popFront() const noexcept
  {
    assert(!isEmpty() && "View was empty!");
    ++begin_ptr;
    --size;
  }

  template<typename T>
  constexpr void ContiguousView<T>::popFrontN(size_t N) const noexcept
  {
    assert(N <= size && "View was empty!");
    begin_ptr += N;
    size -= N;
  }

  template<typename T>
  constexpr void ContiguousView<T>::popBack() const noexcept
  {
    assert(!isEmpty() && "View was empty!");
    --size;
  }

  template<typename T>
  constexpr void ContiguousView<T>::popBackN(size_t N) const noexcept
  {
    assert(N <= size && "View was empty!");
    size -= N;
  }

  template<typename T>
  constexpr copy_if_trivial_t<T> ContiguousView<T>::operator[](size_t index) const noexcept
  {
    assert(index < size && "Invalid index!");
    return begin_ptr[index];
  }

  template<typename T>
  constexpr ContiguousView<T> ContiguousView<T>::spliceRange(Range range) const noexcept
  {
    size_t begin = range.getBeginOffset();
    size_t end = range.getEndOffset();
    end = (end == RangeEndT::value ? size : end);
    
    assert(end < size && "End of range should be smaller than size of view!");

    return { begin_ptr + begin, end - begin };
  }
}

#endif //!HG_COLT_VIEW