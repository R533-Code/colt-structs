/** @file Vector.h
* Contains Vector<>, SmallVector<> and StaticVector<>.
* A Vector<> is a dynamic contiguous array of object that supports
* pushing and popping items at its end.
* A Vector makes use of the global allocator to request memory.
* A SmallVector is a Vector that provides an in-place (stack allocated)
* buffer of small capacity. SmallVectors can be faster than Vectors
* as they can avoid memory allocations for small counts of elements.
* A StaticVector is a Vector that never allocates, storing all its
* items in-place (on the stack). As such, it has a finite capacity,
* and will not be able to store more elements.
*/

#ifndef HG_COLT_VECTOR
#define HG_COLT_VECTOR

#include <initializer_list>

#include "View.h"
#include "../details/allocator.h"
#include "../details/algorithm.h"

namespace colt
{
  /// @brief ContiguousView should at least contain an item
#define colt_vector_is_not_empty this->is_not_empty()
  /// @brief ContigousView should contain at least N items
#define colt_vector_contains_N_item N <= size
  /// @brief 'index' was greater or equal to size of ContiguousView
#define colt_vector_index_smaller_size index < size

  template<typename T>
  /// @brief Contiguous dynamic array
  /// @tparam T The type to store
  class Vector
  {
    static_assert(!traits::is_tag_v<T>, "Cannot use tag struct as typename!");

    /// @brief The TypedBlock owned and managed by the Vector
    memory::TypedBlock<T> blk = { nullptr, 0 };
    /// @brief The count of active objects
    size_t size = 0;

  public:
    /// @brief Default constructs an empty Vector (no allocation)
    constexpr Vector() noexcept = default;

    /// @brief Constructs a Vector with 'reserve' object reserved
    /// @param reserve The count of object to reserve
    constexpr explicit Vector(size_t reserve) noexcept;

    template<typename... Args>
    /// @brief Constructs and fills a Vector of 'fill_size' by forwarding 'args' to the constructor
    /// @tparam ...Args The parameter pack
    /// @param fill_size The count of object to reserve
    /// @param  InPlaceT tag
    /// @param ...args The argument pack
    constexpr Vector(size_t fill_size, traits::InPlaceT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>);

    /// @brief Constructs a Vector from an initializer list.
    /// Only works for copyable T.
    /// @param list The list from which to copy the object
    constexpr Vector(std::initializer_list<T> list)
      noexcept(std::is_nothrow_copy_constructible_v<T>);

    /// @brief Constructs a Vector from a view.
    /// @param view The view whose items to copy
    constexpr Vector(ContiguousView<T> view)
      noexcept(std::is_nothrow_copy_constructible_v<T>);      

    /// @brief Copy constructor.
    /// Only works for copyable T.
    /// @param to_copy Vector whose resources to copy
    constexpr Vector(const Vector& to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>);

    /// @brief Copy assignment operator.
    /// Only works for copyable T.
    /// @param to_copy Vector whose resources to copy
    /// @return Self
    constexpr Vector& operator=(const Vector& to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>);    

    /// @brief Move constructor
    /// @param to_move Vector whose resources to steal
    constexpr Vector(Vector&& to_move) noexcept;      

    /// @brief Move assignment operator.
    /// @param to_move Vector whose resources to steal
    /// @return Self
    constexpr Vector& operator=(Vector&& to_move) noexcept;    

    /// @brief Destructor
    ~Vector()
      noexcept(std::is_nothrow_destructible_v<T>);

    /// @brief Returns an iterator to the beginning of the Vector
    /// @return Iterator to the beginning
    constexpr ContiguousIterator<T> begin() noexcept { return blk.get_ptr(); }
    /// @brief Returns a iterator to the end of the Vector
    /// @return Iterator to the end
    constexpr ContiguousIterator<T> end() noexcept { return blk.get_ptr() + size; }

    /// @brief Returns a const iterator to the beginning of the Vector
    /// @return Const iterator to the beginning
    constexpr ContiguousIterator<const T> begin() const noexcept { return blk.get_ptr(); }
    /// @brief Returns a const iterator to the end of the Vector
    /// @return Const iterator to the end
    constexpr ContiguousIterator<const T> end() const noexcept { return blk.get_ptr() + size; }

    /// @brief Returns a colt iterator over the view
    /// @return Iterator over the whole view
    constexpr iter::ContiguousView<const T> to_iter() const noexcept { return { blk.get_ptr(), size }; }

    /// @brief Returns a pointer to the beginning of the data
    /// @return Const pointer to the beginning of the data
    constexpr const T* get_data() const noexcept { return blk.get_ptr(); }
    /// @brief Returns a pointer to the beginning of the data
    /// @return Pointer to the beginning of the data
    constexpr T* get_data() noexcept { return blk.get_ptr(); }

    /// @brief Returns the count of active objects in the Vector
    /// @return The count of objects in the Vector
    constexpr size_t get_size() const noexcept { return size; }
    /// @brief Returns the capacity of the current allocation
    /// @return The capacity of the current allocation
    constexpr size_t get_capacity() const noexcept { return blk.get_size(); }
    
    /// @brief Returns the byte size of the allocation
    /// @return ByteSize of the allocation
    constexpr sizes::ByteSize get_byte_size() const noexcept { return blk.get_byte_size(); }

    /// @brief Returns the object at index 'index' of the Vector.
    /// @param index The index of the object
    /// @return The object at index 'index'
    /// @pre index < get_size() (colt_vector_index_smaller_size)
    constexpr traits::copy_if_trivial_t<const T&> operator[](size_t index) const noexcept;    

    /// @brief Returns a reference to the object at index 'index' of the Vector.
    /// @param index The index of the object
    /// @return The object at index 'index'
    /// @pre index < get_size() (colt_vector_index_smaller_size)
    constexpr T& operator[](size_t index) noexcept;

    /// @brief Check if the Vector does not contain any object.
    /// Same as: get_size() == 0
    /// @return True if the Vector is empty
    constexpr bool is_empty() const noexcept { return size == 0; }

    /// @brief Check if the Vector does not contain any object.
    /// Same as: get_size() != 0
    /// @return True if the Vector is not empty
    constexpr bool is_not_empty() const noexcept { return size != 0; }

    /// @brief Reserve 'by_more' object
    /// @param by_more The count of object to reserve for
    constexpr void reserve(size_t by_more)
      noexcept(std::is_nothrow_move_constructible_v<T>
        && std::is_nothrow_destructible_v<T>);

    /// @brief Push an object at the end of the Vector by copying
    /// @param to_copy The object to copy at the end of the Vector
    constexpr void push_back(traits::copy_if_trivial_t<const T&> to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>);

    template<typename T_ = T, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    /// @brief Push an object at the end of the Vector by moving
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param to_move The object to move at the end of the Vector
    constexpr void push_back(T&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>);

    template<typename... Args>
    /// @brief Emplace an object at the end of the Vector
    /// @tparam ...Args The parameter pack
    /// @param  InPlaceT tag
    /// @param ...args The argument pack to forward to the constructor
    constexpr void push_back(traits::InPlaceT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>);

    /// @brief Pops an item from the back of the Vector.
    /// @pre !is_empty() (colt_vector_is_not_empty).
    constexpr void pop_back()
      noexcept(std::is_nothrow_destructible_v<T>);

    /// @brief Pops N item from the back of the Vector.
    /// @param N The number of item to pop from the back
    /// @pre N <= get_size() (colt_vector_contains_N_item)
    constexpr void pop_back_n(size_t N)
      noexcept(std::is_nothrow_destructible_v<T>);

    /// @brief Removes all the item from the Vector.
    /// This does not modify the capacity of the Vector.
    constexpr void clear()
      noexcept(std::is_nothrow_destructible_v<T>);

    /// @brief Returns the first item in the Vector.
    /// @return The first item in the Vector.
    /// @pre !is_empty() (colt_vector_is_not_empty).
    constexpr traits::copy_if_trivial_t<const T&> get_front() const noexcept;
    /// @brief Returns the first item in the Vector.
    /// @return The first item in the Vector.
    /// @pre !is_empty() (colt_vector_is_not_empty).
    constexpr T& get_front() noexcept;

    /// @brief Returns the last item in the Vector.
    /// @return The last item in the Vector.
    /// @pre !is_empty() (colt_vector_is_not_empty).
    constexpr traits::copy_if_trivial_t<const T&> get_back() const noexcept;
    /// @brief Returns the last item in the Vector.
    /// @return The last item in the Vector.
    /// @pre !is_empty() (colt_vector_is_not_empty).
    constexpr T& get_back() noexcept;

    /// @brief Obtains a view over the whole Vector
    /// @return View over the Vector
    constexpr ContiguousView<T> to_view() const noexcept { return { blk.get_ptr(), size }; }

    /// @brief Obtains a view over the 'range' of the Vector.
    /// @param range The range to obtain from the Vector
    /// @return View over 'range' of the Vector
    constexpr ContiguousView<T> to_view(Range range) const noexcept;

    /// @brief Converts a Vector to a view implicitly
    /// @return ContiguousView over the whole Vector
    constexpr explicit operator ContiguousView<T>() const noexcept { return { blk.get_ptr(), size }; }

    /// @brief Check if two Vectors are equal
    /// @param a The first Vector
    /// @param b The Second Vector
    /// @return True if both Vectors are equal
    friend constexpr bool operator==(const Vector& a, const Vector& b) noexcept
    {
      return a.to_view() == b.to_view();
    }
    /// @brief Check if two Vectors are not equal
    /// @param a The first Vector
    /// @param b The Second Vector
    /// @return True if both Vectors are not equal
    friend constexpr bool operator!=(const Vector& a, const Vector& b) noexcept
    {
      return a.to_view() != b.to_view();
    }
  };

  template<typename T, size_t buff_count = 5>
  /// @brief A Vector with a small buffer optimization.
  /// A SmallVector is a vector containing an internal stack buffer
  /// that can contain up to 'buff_count' elements.
  /// This allows to avoid memory allocations for small counts of elements.
  /// This can be a pessimization if the SmallVector regularly contains more
  /// than 'buff_count' elements: the internal stack buffer occupies memory
  /// that will not be used.
  /// Using a SmallVector also adds a branch for every operation done on it:
  /// it needs to check if the SmallVector is using its internal buffer.
  /// @tparam T The type to store
  class SmallVector
  {
    static_assert(!traits::is_tag_v<T>, "Cannot use tag struct as typename!");
    static_assert(buff_count != 0, "Use Vector for buff_count == 0!");

    union
    {
      /// @brief Pointer to the allocation
      T* ptr;
      /// @brief Stack allocated buffer holding the stack allocated objects
      alignas(alignof(T)) char buffer[sizeof(T) * buff_count];
    };
    
    /// @brief The capacity of the allocation or buff_count if stack allocated
    size_t capacity = buff_count;
    /// @brief The count of active objects
    size_t size = 0;

  private:
    /// @brief Returns the stack buffer's beginning pointer
    /// @return Const pointer to the stack buffer
    constexpr const T* get_stack_ptr() const noexcept { return std::launder(reinterpret_cast<const T*>(buffer)); }
    /// @brief Returns the stack buffer's beginning pointer
    /// @return Pointer to the stack buffer
    constexpr T* get_stack_ptr() noexcept { return std::launder(reinterpret_cast<T*>(buffer)); }

    /// @brief Returns the pointer to the current active allocation
    /// @return Const pointer to the current active allocation
    constexpr const T* get_current_ptr() const noexcept;
    /// @brief Returns the pointer to the current active allocation
    /// @return Pointer to the current active allocation
    constexpr T* get_current_ptr() noexcept;

  public:
    /// @brief Default constructor
    constexpr SmallVector() noexcept = default;

    /// @brief Creates a SmallVector with a capacity of 'reserve_size'.
    /// If 'reserve_size' is smaller than 'buff_count', then the capacity
    /// is set to 'buff_count'
    /// @param reserve_size The capacity of the SmallVector
    explicit constexpr SmallVector(size_t reserve_size) noexcept;      

    template<typename... Args>
    /// @brief Constructs and fills a Vector of 'fill_size' by forwarding 'args' to the constructor
    /// @tparam ...Args The parameter pack
    /// @param fill_size The count of object to reserve
    /// @param  InPlaceT tag
    /// @param ...args The argument pack
    constexpr SmallVector(size_t fill_size, traits::InPlaceT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>);

    /// @brief Initializer list constructor
    /// @param list The list whose items to copy
    constexpr SmallVector(std::initializer_list<T> list)
      noexcept(std::is_nothrow_copy_constructible_v<T>);

    /// @brief Copy constructor
    /// @param to_copy The SmallVector whose data to copy
    constexpr SmallVector(const SmallVector& to_copy)
      noexcept(std::is_nothrow_destructible_v<T>
        && std::is_nothrow_copy_constructible_v<T>);

    /// @brief Move constructor
    /// @param to_move The SmallVector whose data to move
    constexpr SmallVector(SmallVector&& to_move)
      noexcept(std::is_nothrow_destructible_v<T>
        && std::is_nothrow_move_constructible_v<T>);

    /// @brief Move assignment operator
    /// @param to_move The SmallVector whose data to move
    /// @return Self
    constexpr SmallVector& operator=(SmallVector&& to_move)
      noexcept(std::is_nothrow_destructible_v<T>
        && std::is_nothrow_move_constructible_v<T>);

    /// @brief Copy assignment operator
    /// @param to_copy The SmallVector whose data to copy
    /// @return Self
    constexpr SmallVector& operator=(const SmallVector& to_copy)
      noexcept(std::is_nothrow_destructible_v<T>
        && std::is_nothrow_copy_constructible_v<T>);

    /// @brief Destructor
    ~SmallVector()
      noexcept(std::is_nothrow_destructible_v<T>);

    /// @brief Returns an iterator to the beginning of the Vector
    /// @return Iterator to the beginning
    constexpr ContiguousIterator<T> begin() noexcept { return get_current_ptr(); }
    /// @brief Returns a iterator to the end of the Vector
    /// @return Iterator to the end
    constexpr ContiguousIterator<T> end() noexcept { return get_current_ptr() + size; }

    /// @brief Returns a const iterator to the beginning of the Vector
    /// @return Const iterator to the beginning
    constexpr ContiguousIterator<const T> begin() const noexcept { return get_current_ptr(); }
    /// @brief Returns a const iterator to the end of the Vector
    /// @return Const iterator to the end
    constexpr ContiguousIterator<const T> end() const noexcept { return get_current_ptr() + size; }

    /// @brief Returns a colt iterator over the view
    /// @return Iterator over the whole view
    constexpr iter::ContiguousView<const T> to_iter() const noexcept { return { get_current_ptr(), size }; }

    /// @brief Returns a pointer to the beginning of the data
    /// @return Const pointer to the beginning of the data
    constexpr const T* get_data() const noexcept { return get_current_ptr(); }
    /// @brief Returns a pointer to the beginning of the data
    /// @return Pointer to the beginning of the data
    constexpr T* get_data() noexcept { return get_current_ptr(); }

    /// @brief Check if the SmallVector is empty
    /// @return True if empty
    constexpr bool is_empty() const noexcept { return size == 0; }
    /// @brief Check if the SmallVector is not empty
    /// @return True if not empty
    constexpr bool is_not_empty() const noexcept { return size != 0; }

    /// @brief The count of active elements in the SmallVector
    /// @return The count of active elements in the SmallVector
    constexpr size_t get_size() const noexcept { return size; }
    /// @brief Returns the capacity of the current allocation.
    /// If the SmallVector is currently using stack memory, returns 'buff_count'
    /// @return Capacity of the current allocation or 'buff_count'
    constexpr size_t get_capacity() const noexcept { return capacity; }

    /// @brief Check if the SmallVector is using its internal stack-allocated buffer
    /// @return True if the elements are stored in the stack-allocated buffer
    constexpr bool is_stack_allocated() const noexcept { return capacity == buff_count; }

    /// @brief Returns the byte size of the allocation
    /// @return ByteSize of the allocation
    constexpr sizes::ByteSize get_byte_size() const noexcept { return { capacity * sizeof(T) }; }

    /// @brief Returns the object at index 'index' of the Vector.
    /// @param index The index of the object
    /// @return The object at index 'index'
    /// @pre index < get_size() (colt_vector_index_smaller_size).
    constexpr traits::copy_if_trivial_t<const T&> operator[](size_t index) const noexcept;

    /// @brief Returns a reference to the object at index 'index' of the Vector.
    /// @param index The index of the object
    /// @return The object at index 'index'
    /// @pre index < get_size() (colt_vector_index_smaller_size).
    constexpr T& operator[](size_t index) noexcept;

    /// @brief Push an object at the end of the Vector by copying
    /// @param to_copy The object to copy at the end of the Vector
    constexpr void push_back(traits::copy_if_trivial_t<const T&> to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>);

    template<typename T_ = T, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    /// @brief Push an object at the end of the Vector by moving
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param to_move The object to move at the end of the Vector
    constexpr void push_back(T&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>);    

    template<typename... Args>
    /// @brief Emplace an object at the end of the Vector
    /// @tparam ...Args The parameter pack
    /// @param  InPlaceT tag
    /// @param ...args The argument pack to forward to the constructor
    constexpr void push_back(traits::InPlaceT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>);    

    /// @brief Pops an item from the back of the Vector.
    /// @pre is_not_empty() (colt_vector_is_not_empty).
    constexpr void pop_back()
      noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Pops N item from the back of the Vector.
    /// @param N The number of item to pop from the back
    /// @pre N <= get_size() (colt_vector_contains_N_item).
    constexpr void pop_back_n(size_t N)
      noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Removes all the item from the Vector.
    /// This does not modify the capacity of the Vector.
    constexpr void clear()
      noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Returns the first item in the Vector.
    /// @return The first item in the Vector.
    /// @pre is_not_empty() (colt_vector_is_not_empty).
    constexpr traits::copy_if_trivial_t<const T&> get_front() const noexcept;    
    /// @brief Returns the first item in the Vector.
    /// @return The first item in the Vector.
    /// @pre is_not_empty() (colt_vector_is_not_empty).
    constexpr T& get_front() noexcept;    

    /// @brief Returns the last item in the Vector.
    /// @return The last item in the Vector.
    /// @pre is_not_empty() (colt_vector_is_not_empty).
    constexpr traits::copy_if_trivial_t<const T&> get_back() const noexcept;    
    /// @brief Returns the last item in the Vector.
    /// @return The last item in the Vector.
    /// @pre is_not_empty() (colt_vector_is_not_empty).
    constexpr T& get_back() noexcept;    

    /// @brief Reserves 'by_more' capacity.
    /// This always causes an allocation, so be sure that the stack capacity is not
    /// enough before calling.
    /// @param by_more The count of object to reserve for
    constexpr void reserve(size_t by_more)
      noexcept(std::is_nothrow_move_constructible_v<T>
        && std::is_nothrow_destructible_v<T>);

    /// @brief Obtains a view over the whole Vector
    /// @return View over the Vector
    constexpr ContiguousView<T> to_view() const noexcept { return { get_current_ptr(), size}; }
    
    /// @brief Obtains a view over the 'range' of the Vector.
    /// @param range The range to obtain from the Vector
    /// @return View over 'range' of the Vector
    constexpr ContiguousView<T> to_view(Range range) const noexcept;

    /// @brief Converts a Vector to a view implicitly
    /// @return ContiguousView over the whole Vector
    constexpr explicit operator ContiguousView<T>() const noexcept { return { get_current_ptr(), size }; }  

    /// @brief Check if two SmallVectors are equal
    /// @param a The first SmallVector
    /// @param b The second SmallVector
    /// @return True if equal
    friend constexpr bool operator==(const SmallVector& a, const SmallVector& b) noexcept
    {      
      return a.to_view() == b.to_view();
    }

    /// @brief Check if two SmallVectors are not equal
    /// @param a The first SmallVector
    /// @param b The second SmallVector
    /// @return True if not equal
    friend constexpr bool operator!=(const SmallVector& a, const SmallVector& b) noexcept
    {
      return a.to_view() != b.to_view();
    }
  };

  template<typename T, size_t max_size>
  /// @brief A Vector with a small buffer optimization.
  /// A SmallVector is a vector containing an internal stack buffer
  /// that can contain up to 'buff_count' elements.
  /// This allows to avoid memory allocations for small counts of elements.
  /// This can be a pessimization if the SmallVector regularly contains more
  /// than 'buff_count' elements: the internal stack buffer occupies memory
  /// that will not be used.
  /// @tparam T The type to store
  class StaticVector
  {
    static_assert(!traits::is_tag_v<T>, "Cannot use tag struct as typename!");
    static_assert(max_size != 0, "Invalid max size!");

    /// @brief The count of active objects
    size_t size = 0;
    /// @brief Stack buffer for the objects
    alignas(alignof(T)) char buffer[sizeof(T) * max_size];

  private:
    /// @brief Returns the stack buffer's beginning pointer
    /// @return Const pointer to the stack buffer
    constexpr const T* get_ptr() const noexcept { return std::launder(reinterpret_cast<const T*>(buffer)); }
    /// @brief Returns the stack buffer's beginning pointer
    /// @return Pointer to the stack buffer
    constexpr T* get_ptr() noexcept { return std::launder(reinterpret_cast<T*>(buffer)); }

  public:
    /// @brief Default constructor
    constexpr StaticVector() noexcept {}

    /// @brief Copy constructor
    /// @param to_copy The StaticVector to copy
    constexpr StaticVector(const StaticVector& to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>);
    
    /// @brief Move constructor
    /// @param to_move The StaticVector to copy
    constexpr StaticVector(StaticVector&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>);

    /// @brief Move assignment operator
    /// @param to_move The StaticVector to move
    /// @return Self
    constexpr StaticVector& operator=(StaticVector&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>
        && std::is_nothrow_destructible_v<T>);

    /// @brief Copy assignment operator
    /// @param to_copy The StaticVector whose data to copy
    /// @return Self
    constexpr StaticVector& operator=(const StaticVector& to_copy)
      noexcept(std::is_nothrow_destructible_v<T>
        && std::is_nothrow_copy_constructible_v<T>);

    /// @brief Destructor
    ~StaticVector()
      noexcept(std::is_nothrow_destructible_v<T>);

    /// @brief Returns an iterator to the beginning of the StaticVector
    /// @return iterator to the beginning
    constexpr ContiguousIterator<T> begin() noexcept { return get_ptr(); }
    /// @brief Returns an iterator to the end of the StaticVector
    /// @return iterator to the end
    constexpr ContiguousIterator<T> end() noexcept { return get_ptr() + size; }

    /// @brief Returns a const iterator to the beginning of the StaticVector
    /// @return Const iterator to the beginning
    constexpr ContiguousIterator<const T> begin() const noexcept { return get_ptr(); }
    /// @brief Returns a const iterator to the end of the StaticVector
    /// @return Const iterator to the end
    constexpr ContiguousIterator<const T> end() const noexcept { return get_ptr() + size; }

    /// @brief Returns a colt iterator over the view
    /// @return Iterator over the whole view
    constexpr iter::ContiguousView<const T> to_iter() const noexcept { return { get_ptr(), size }; }

    /// @brief Returns a pointer to the beginning of the data
    /// @return Const pointer to the beginning of the data
    constexpr const T* get_data() const noexcept { return get_ptr(); }
    /// @brief Returns a pointer to the beginning of the data
    /// @return Pointer to the beginning of the data
    constexpr T* get_data() noexcept { return get_ptr(); }

    /// @brief Check if the StaticVector does not contain any object.
    /// Same as: get_size() == 0
    /// @return True if the StaticVector is empty
    constexpr bool is_empty() const noexcept { return size == 0; }
    /// @brief Check if the StaticVector contains any object.
    /// Same as: get_size() != 0
    /// @return True if the StaticVector is not empty
    constexpr bool is_not_empty() const noexcept { return size != 0; }

    /// @brief Check if the StaticVector is full
    /// @return True if the StaticVector is full
    constexpr bool is_full() const noexcept { return size == max_size; }
    /// @brief Check if the StaticVector can still contain objects
    /// @return True if the StaticVector can still contain objects
    constexpr bool is_not_full() const noexcept { return size == max_size; }

    /// @brief Returns the count of active objects in the StaticVector
    /// @return The count of objects in the StaticVector
    constexpr size_t get_size() const noexcept { return size; }
    /// @brief Returns the maximum count of active objects
    /// @return 'max_size', the maximum count of active objects
    constexpr size_t get_capacity() const noexcept { return max_size; }

    /// @brief Returns the byte size of the internal buffer
    /// @return ByteSize of the internal buffer
    constexpr sizes::ByteSize get_byte_size() const noexcept { return { max_size * sizeof(T) }; }

    /// @brief Returns the object at index 'index' of the StaticVector.
    /// @param index The index of the object
    /// @return The object at index 'index'
    /// @pre index < get_size() (colt_vector_index_smaller_size)
    constexpr traits::copy_if_trivial_t<const T&> operator[](size_t index) const noexcept;
    /// @brief Returns a reference to the object at index 'index' of the StaticVector.
    /// @param index The index of the object
    /// @return The object at index 'index'
    /// @pre index < get_size() (colt_vector_index_smaller_size).
    constexpr T& operator[](size_t index) noexcept;

    /// @brief Push an object at the end of the StaticVector by copying
    /// @param to_copy The object to copy at the end of the StaticVector
    /// @return True if push_back was successful, false if full
    constexpr bool push_back(traits::copy_if_trivial_t<const T&> to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>);

    template<typename T_ = T, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    /// @brief Push an object at the end of the StaticVector by moving
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param to_move The object to move at the end of the StaticVector
    /// @return True if push_back was successful, false if full
    constexpr bool push_back(T&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>);

    template<typename... Args>
    /// @brief Emplace an object at the end of the StaticVector
    /// @tparam ...Args The parameter pack
    /// @param  InPlaceT tag
    /// @param ...args The argument pack to forward to the constructor
    /// @return True if push_back was successful, false if full
    constexpr bool push_back(traits::InPlaceT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>);

    /// @brief Pops an item from the back of the StaticVector.
    /// @pre !is_empty() (colt_vector_is_not_empty).
    constexpr void pop_back() noexcept(std::is_nothrow_destructible_v<T>);
    /// @brief Pops N item from the back of the StaticVector.
    /// @param N The number of item to pop from the back
    /// @pre N <= get_size() (colt_vector_contains_N_item)
    constexpr void pop_back_n(size_t N) noexcept(std::is_nothrow_destructible_v<T>);

    /// @brief Removes all the active items from the StaticVector.
    constexpr void clear()
      noexcept(std::is_nothrow_destructible_v<T>);

    /// @brief Returns the first item in the StaticVector.
    /// @return The first item in the StaticVector.
    /// @pre !is_empty() (colt_vector_is_not_empty).
    constexpr traits::copy_if_trivial_t<const T&> get_front() const noexcept;
    /// @brief Returns the first item in the StaticVector.
    /// @return The first item in the StaticVector.
    /// @pre !is_empty() (colt_vector_is_not_empty).
    constexpr T& get_front() noexcept;

    /// @brief Returns the last item in the StaticVector.
    /// @return The last item in the StaticVector.
    /// @pre !is_empty() (colt_vector_is_not_empty).
    constexpr traits::copy_if_trivial_t<const T&> get_back() const noexcept;
    /// @brief Returns the last item in the StaticVector.
    /// @return The last item in the StaticVector.
    /// @pre !is_empty() (colt_vector_is_not_empty).
    constexpr T& get_back() noexcept;

    /// @brief Obtains a view over the whole StaticVector
    /// @return View over the StaticVector
    constexpr ContiguousView<T> to_view() const noexcept { return { get_ptr(), size }; }
    /// @brief Obtains a view over the 'range' of the StaticVector.
    /// @param range The range to obtain from the StaticVector
    /// @return View over 'range' of the StaticVector
    constexpr ContiguousView<T> to_view(Range range) const noexcept;

    /// @brief Converts a StaticVector to a view implicitly
    /// @return ContiguousView over the whole StaticVector
    constexpr explicit operator ContiguousView<T>() const noexcept { return { get_ptr(), size }; }

    /// @brief Check if two StaticVectors are equal
    /// @param a The first StaticVector
    /// @param b The Second StaticVector
    /// @return True if both StaticVectors are equal
    friend constexpr bool operator==(const StaticVector& a, const StaticVector& b) noexcept
    {
      return a.to_view() == b.to_view();
    }

    /// @brief Check if two StaticVectors are not equal
    /// @param a The first StaticVector
    /// @param b The Second StaticVector
    /// @return True if both StaticVectors are not equal
    friend constexpr bool operator!=(const StaticVector& a, const StaticVector& b) noexcept
    {
      return a.to_view() != b.to_view();
    }
  };

  template<typename T>
  constexpr Vector<T>::Vector(size_t reserve) noexcept
    : blk(memory::allocate({ reserve * sizeof(T) })) {}

  template<typename T>
  template<typename ...Args>
  constexpr Vector<T>::Vector(size_t fill_size, traits::InPlaceT, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args ...>)
    : blk(memory::allocate({ fill_size * sizeof(T) })), size(fill_size)
  {
    algo::contiguous_construct(blk.get_ptr(), size, std::forward<Args>(args)...);
  }

  template<typename T>
  constexpr Vector<T>::Vector(std::initializer_list<T> list)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
    : blk(memory::allocate({ list.size() * sizeof(T) })), size(list.size())
  {
    algo::contiguous_copy(list.begin(), blk.get_ptr(), size);
  }

  template<typename T>
  constexpr Vector<T>::Vector(ContiguousView<T> view) noexcept(std::is_nothrow_copy_constructible_v<T>)
    : blk(memory::allocate({ view.get_size() * sizeof(T) })), size(view.get_size())
  {
    algo::contiguous_copy(view.begin(), blk.get_ptr(), size);
  }

  template<typename T>
  constexpr Vector<T>::Vector(const Vector& to_copy)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
    : blk(memory::allocate(to_copy.blk.get_byte_size())), size(to_copy.get_size())
  {
    algo::contiguous_copy(to_copy.begin(), blk.get_ptr(), size);
  }

  template<typename T>
  constexpr Vector<T>& Vector<T>::operator=(const Vector& to_copy)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
  {
    if (&to_copy == this)
      return *this;

    clear();
    for (size_t i = 0; i < to_copy.size; i++)
      this->push_back(to_copy[i]);

    return *this;
  }

  template<typename T>
  constexpr Vector<T>::Vector(Vector&& to_move) noexcept
    : blk(colt::exchange(to_move.blk, { nullptr, 0 })), size(colt::exchange(to_move.size, 0)) {}

  template<typename T>
  constexpr Vector<T>& Vector<T>::operator=(Vector&& to_move) noexcept
  {
    if (&to_move == this)
      return *this;

    //Swap members
    colt::swap(to_move.blk, blk);
    colt::swap(to_move.size, size);

    return *this;
  }

  template<typename T>
  Vector<T>::~Vector()
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    clear();
    if (blk)
      memory::deallocate(blk);
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> Vector<T>::operator[](size_t index) const noexcept
  {
    CHECK_REQUIREMENT(colt_vector_index_smaller_size);
    return blk.get_ptr()[index];
  }

  template<typename T>
  constexpr T& Vector<T>::operator[](size_t index) noexcept
  {
    CHECK_REQUIREMENT(colt_vector_index_smaller_size);
    return blk.get_ptr()[index];
  }

  template<typename T>
  constexpr void Vector<T>::reserve(size_t by_more)
    noexcept(std::is_nothrow_move_constructible_v<T>
      && std::is_nothrow_destructible_v<T>)
  {
    memory::TypedBlock<T> new_blk = memory::allocate({ blk.get_byte_size().size + by_more * sizeof(T) });
    
    algo::contiguous_destructive_move(blk.get_ptr(), new_blk.get_ptr(), size);

    memory::deallocate(blk);
    blk = new_blk;
  }

  template<typename T>
  constexpr void Vector<T>::push_back(traits::copy_if_trivial_t<const T&> to_copy)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
  {
    if (size == blk.get_size())
      reserve(blk.get_size() + 4);
    new(blk.get_ptr() + size) T(to_copy);
    ++size;
  }

  template<typename T>
  constexpr void Vector<T>::pop_back()
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    --size;
    blk.get_ptr()[size].~T();
  }

  template<typename T>
  constexpr void Vector<T>::pop_back_n(size_t N)
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    CHECK_REQUIREMENT(colt_vector_contains_N_item);
    for (size_t i = size - N; i < size; i++)
      blk.get_ptr()[i].~T();
    size -= N;
  }

  template<typename T>
  constexpr void Vector<T>::clear()
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    algo::contiguous_destruct(blk.get_ptr(), size);
    size = 0;
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> Vector<T>::get_front() const noexcept
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    return *blk.get_ptr();
  }

  template<typename T>
  constexpr T& Vector<T>::get_front() noexcept
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    return *blk.get_ptr();
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> Vector<T>::get_back() const noexcept
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    return blk.get_ptr()[size - 1];
  }

  template<typename T>
  constexpr T& Vector<T>::get_back() noexcept
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    return blk.get_ptr()[size - 1];
  }

  template<typename T>
  constexpr ContiguousView<T> Vector<T>::to_view(Range range) const noexcept
  {
    size_t begin = range.get_begin_offset();
    if (begin >= size)
      return {};
    size_t end = range.get_end_offset();
    end = (end > size ? size : end);
    return { get_data() + begin, end - begin };
  }  
  
  template<typename T>
  template<typename T_, typename>
  constexpr void Vector<T>::push_back(T&& to_move)
    noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    if (size == get_capacity())
      reserve(get_capacity() + 16);
    new(blk.get_ptr() + size) T(std::move(to_move));
    ++size;
  }
    
  template<typename T>
  template<typename ...Args>
  constexpr void Vector<T>::push_back(traits::InPlaceT, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args ...>)
  {
    if (size == get_capacity())
      reserve(get_capacity() + 16);
    new(blk.get_ptr() + size) T(std::forward<Args>(args)...);
    ++size;
  }

  template<typename T, size_t buff_count>
  constexpr SmallVector<T, buff_count>::SmallVector(size_t reserve_size) noexcept
    : capacity(reserve_size <= buff_count ? buff_count : reserve_size)
  {
    //make the allocation pointer active
    if (!is_stack_allocated())
      ptr = reinterpret_cast<T*>(memory::allocate({ capacity * sizeof(T) }).get_ptr());
  }

  template<typename T, size_t buff_count>
  constexpr SmallVector<T, buff_count>::SmallVector(std::initializer_list<T> list)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
    : capacity(std::size(list) <= buff_count ? buff_count : std::size(list)), size(std::size(list))
  {
    if (!is_stack_allocated())
    {
      ptr = reinterpret_cast<T*>(memory::allocate({ capacity * sizeof(T) }).get_ptr());
      algo::contiguous_copy(list.begin(), ptr, size);
    }
    else
    {
      algo::contiguous_copy(list.begin(), get_stack_ptr(), size);
    }
  }

  template<typename T, size_t buff_count>
  constexpr SmallVector<T, buff_count>::SmallVector(const SmallVector& to_copy)
    noexcept(std::is_nothrow_destructible_v<T>
      && std::is_nothrow_copy_constructible_v<T>)
    : capacity(to_copy.capacity), size(to_copy.size)
  {
    if (!is_stack_allocated())
    {
      ptr = reinterpret_cast<T*>(memory::allocate({ capacity * sizeof(T) }).get_ptr());
      algo::contiguous_copy(to_copy.ptr, ptr, size);
    }
    else
    {
      T* const to = get_stack_ptr();
      const T* const from = to_copy.get_stack_ptr();
      algo::contiguous_copy(from, to, size);
    }
  }

  template<typename T, size_t buff_count>
  constexpr SmallVector<T, buff_count>::SmallVector(SmallVector&& to_move)
    noexcept(std::is_nothrow_destructible_v<T>
      && std::is_nothrow_move_constructible_v<T>)
    : capacity(colt::exchange(to_move.capacity, buff_count))
    , size(colt::exchange(to_move.size, 0))
  {
    if (!is_stack_allocated())
      ptr = colt::exchange(to_move.ptr, nullptr);
    else
      algo::contiguous_destructive_move(to_move.get_stack_ptr(), get_stack_ptr(), size);
  }

  template<typename T, size_t buff_count>
  constexpr SmallVector<T, buff_count>& SmallVector<T, buff_count>::operator=(SmallVector&& to_move)
    noexcept(std::is_nothrow_destructible_v<T>
      && std::is_nothrow_move_constructible_v<T>)
  {
    if (&to_move == this)
      return *this;
    
    clear();
    if (!is_stack_allocated())
      memory::deallocate({ ptr, capacity * sizeof(T) });
    
    if (to_move.is_stack_allocated())
    {
      capacity = buff_count;
      algo::contiguous_destructive_move(to_move.get_stack_ptr(), get_stack_ptr(), to_move.size);
    }
    else
    {
      capacity = colt::exchange(to_move.capacity, buff_count);
      ptr = colt::exchange(to_move.ptr, nullptr);
    }
    size = colt::exchange(to_move.size, 0);
  }

  template<typename T, size_t buff_count>
  constexpr SmallVector<T, buff_count>& SmallVector<T, buff_count>::operator=(const SmallVector& to_copy)
    noexcept(std::is_nothrow_destructible_v<T>
      && std::is_nothrow_copy_constructible_v<T>)
  {
    if (&to_copy == this)
      return *this;
    
    clear();
    if (capacity >= to_copy.capacity) //no need to allocate
    {
      algo::contiguous_copy(to_copy.get_current_ptr(), get_current_ptr(), to_copy.size);
      size = to_copy.size;
    }
    else //not stack allocated
    {
      if (!is_stack_allocated())
        memory::deallocate({ ptr, capacity * sizeof(T) });
      
      memory::TypedBlock<T> blk = memory::allocate({ sizeof(T) * to_copy.capacity });
      capacity = blk.get_size();
      ptr = blk.get_ptr();
      algo::contiguous_copy(to_copy.ptr, ptr, to_copy.size);
      size = to_copy.size;
    }
  }

  template<typename T, size_t buff_count>
  SmallVector<T, buff_count>::~SmallVector() noexcept(std::is_nothrow_destructible_v<T>)
  {
    clear();
    if (!is_stack_allocated())
      memory::deallocate({ ptr, capacity * sizeof(T) });
  }

  template<typename T, size_t buff_count>
  constexpr traits::copy_if_trivial_t<const T&> SmallVector<T, buff_count>::operator[](size_t index) const noexcept
  {
    CHECK_REQUIREMENT(colt_vector_index_smaller_size);
    return get_current_ptr()[index];
  }

  template<typename T, size_t buff_count>
  constexpr T& SmallVector<T, buff_count>::operator[](size_t index) noexcept
  {
    CHECK_REQUIREMENT(colt_vector_index_smaller_size);
    return get_current_ptr()[index];
  }

  template<typename T, size_t buff_count>
  constexpr void SmallVector<T, buff_count>::push_back(traits::copy_if_trivial_t<const T&> to_copy)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
  {
    if (size == capacity)
      reserve(capacity);
    new(get_current_ptr() + size) T(to_copy);
    ++size;
  }

  template<typename T, size_t buff_count>
  constexpr void SmallVector<T, buff_count>::pop_back()
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    --size;
    get_current_ptr()[size].~T();
  }

  template<typename T, size_t buff_count>
  constexpr void SmallVector<T, buff_count>::pop_back_n(size_t N)
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    CHECK_REQUIREMENT(colt_vector_contains_N_item);
    T* const ptr_d = get_current_ptr();
    algo::contiguous_destruct(ptr_d + (size -= N), N);
  }

  template<typename T, size_t buff_count>
  constexpr void SmallVector<T, buff_count>::clear()
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    T* const ptr_d = get_current_ptr();
    algo::contiguous_destruct(ptr_d, size);
    size = 0;
  }

  template<typename T, size_t buff_count>
  constexpr traits::copy_if_trivial_t<const T&> SmallVector<T, buff_count>::get_front() const noexcept
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    return *get_current_ptr();
  }

  template<typename T, size_t buff_count>
  constexpr T& SmallVector<T, buff_count>::get_front() noexcept
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    return *get_current_ptr();
  }

  template<typename T, size_t buff_count>
  constexpr traits::copy_if_trivial_t<const T&> SmallVector<T, buff_count>::get_back() const noexcept
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    return get_current_ptr()[size - 1];
  }

  template<typename T, size_t buff_count>
  constexpr T& SmallVector<T, buff_count>::get_back() noexcept
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    return get_current_ptr()[size - 1];
  }

  template<typename T, size_t buff_count>
  constexpr void SmallVector<T, buff_count>::reserve(size_t by_more)
    noexcept(std::is_nothrow_move_constructible_v<T>
      && std::is_nothrow_destructible_v<T>)
  {
    memory::TypedBlock<T> blk = memory::allocate({ sizeof(T) * (capacity + by_more) });
    T* const ptr_d = get_current_ptr();
    
    algo::contiguous_destructive_move(ptr_d, blk.get_ptr(), size);
    
    if (!is_stack_allocated())
      memory::deallocate({ ptr_d, capacity * sizeof(T) });
    capacity += by_more;
    ptr = blk.get_ptr();
  }

  template<typename T, size_t buff_count>
  constexpr ContiguousView<T> SmallVector<T, buff_count>::to_view(Range range) const noexcept
  {
    size_t begin = range.get_begin_offset();
    if (begin >= size)
      return {};
    size_t end = range.get_end_offset();
    end = (end > size ? size : end);
    return { get_current_ptr() + begin, end - begin};
  }

  template<typename T, size_t buff_count>
  constexpr const T* SmallVector<T, buff_count>::get_current_ptr() const noexcept
  {
    if (is_stack_allocated())
      return get_stack_ptr();
    return ptr;
  }

  template<typename T, size_t buff_count>
  constexpr T* SmallVector<T, buff_count>::get_current_ptr() noexcept
  {
    if (is_stack_allocated())
      return get_stack_ptr();
    return ptr;
  }
  
  template<typename T, size_t buff_count>
  template<typename ...Args>
  constexpr SmallVector<T, buff_count>::SmallVector(size_t fill_size, traits::InPlaceT, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args ...>)
    : capacity(fill_size > buff_count ? fill_size : buff_count), size(fill_size)
  {
    if (is_stack_allocated())
      algo::contiguous_construct(get_stack_ptr(), size, std::forward<Args>(args)...);
    else
    {
      memory::TypedBlock<T> blk = memory::allocate({ sizeof(T) * capacity });
      ptr = blk.get_ptr();
      algo::contiguous_construct(ptr, size, std::forward<Args>(args)...);
    }
  }

  template<typename T, size_t buff_count>
  template<typename T_, typename>
  constexpr void SmallVector<T, buff_count>::push_back(T&& to_move)
    noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    if (size == capacity)
      reserve(capacity);
    new(get_current_ptr() + size) T(std::move(to_move));
    ++size;
  }

  template<typename T, size_t buff_count>
  template<typename ...Args>
  constexpr void SmallVector<T, buff_count>::push_back(traits::InPlaceT, Args && ...args)
    noexcept(std::is_nothrow_constructible_v<T, Args ...>)
  {
    if (size == capacity)
      reserve(capacity);
    new(get_current_ptr() + size) T(std::forward<Args>(args)...);
    ++size;
  }
  
  template<typename T, size_t max_size>
  constexpr StaticVector<T, max_size>::StaticVector(const StaticVector& to_copy)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
    : size(to_copy.size)
  {
    algo::contiguous_copy(to_copy.get_ptr(), get_ptr(), size);
  }

  template<typename T, size_t max_size>
  constexpr StaticVector<T, max_size>::StaticVector(StaticVector&& to_move)
    noexcept(std::is_nothrow_move_constructible_v<T>)
    : size(colt::exchange(to_move.size, 0))
  {
    algo::contiguous_destructive_move(to_move.get_ptr(), get_ptr(), size);
  }

  template<typename T, size_t max_size>
  constexpr StaticVector<T, max_size>& StaticVector<T, max_size>::operator=(StaticVector&& to_move)
    noexcept(std::is_nothrow_move_constructible_v<T>
      && std::is_nothrow_destructible_v<T>)
  {
    if (&to_move == this)
      return *this;

    clear();
    size = colt::exchange(to_move.size, 0);
    algo::contiguous_destructive_move(to_move.get_ptr(), get_ptr(), size);

    return *this;
  }

  template<typename T, size_t max_size>
  constexpr StaticVector<T, max_size>& StaticVector<T, max_size>::operator=(const StaticVector& to_copy)
    noexcept(std::is_nothrow_destructible_v<T>
      && std::is_nothrow_copy_constructible_v<T>)
  {
    if (&to_copy == this)
      return *this;

    clear();
    size = to_copy.size;
    algo::contiguous_copy(to_copy.get_ptr(), get_ptr(), size);

    return *this;
  }

  template<typename T, size_t max_size>
  StaticVector<T, max_size>::~StaticVector()
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    clear();
  }

  template<typename T, size_t max_size>
  constexpr traits::copy_if_trivial_t<const T&> StaticVector<T, max_size>::operator[](size_t index) const noexcept
  {
    CHECK_REQUIREMENT(colt_vector_index_smaller_size);
    return get_ptr()[index];
  }

  template<typename T, size_t max_size>
  constexpr T& StaticVector<T, max_size>::operator[](size_t index) noexcept
  {
    CHECK_REQUIREMENT(colt_vector_index_smaller_size);
    return get_ptr()[index];
  }

  template<typename T, size_t max_size>
  constexpr bool StaticVector<T, max_size>::push_back(traits::copy_if_trivial_t<const T&> to_copy)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
  {
    if (size == max_size)
      return false;
    new(get_ptr() + size++) T(to_copy);
    return true;
  }

  template<typename T, size_t max_size>
  constexpr void StaticVector<T, max_size>::pop_back()
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    --size;
    get_ptr()[size].~T();
  }

  template<typename T, size_t max_size>
  constexpr void StaticVector<T, max_size>::pop_back_n(size_t N)
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    CHECK_REQUIREMENT(colt_vector_contains_N_item);
    T* const ptr_d = get_ptr();
    algo::contiguous_destruct(ptr_d + (size -= N), N);
  }

  template<typename T, size_t max_size>
  constexpr void StaticVector<T, max_size>::clear() noexcept(std::is_nothrow_destructible_v<T>)
  {
    algo::contiguous_destruct(get_ptr(), size);
    size = 0;
  }

  template<typename T, size_t max_size>
  constexpr traits::copy_if_trivial_t<const T&> StaticVector<T, max_size>::get_front() const noexcept
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    return *get_ptr();
  }

  template<typename T, size_t max_size>
  constexpr T& StaticVector<T, max_size>::get_front() noexcept
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    return *get_ptr();
  }

  template<typename T, size_t max_size>
  constexpr traits::copy_if_trivial_t<const T&> StaticVector<T, max_size>::get_back() const noexcept
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    return get_ptr()[size - 1];
  }

  template<typename T, size_t max_size>
  constexpr T& StaticVector<T, max_size>::get_back() noexcept
  {
    CHECK_REQUIREMENT(colt_vector_is_not_empty);
    return get_ptr()[size - 1];
  }

  template<typename T, size_t max_size>
  constexpr ContiguousView<T> StaticVector<T, max_size>::to_view(Range range) const noexcept
  {
    size_t begin = range.get_begin_offset();
    if (begin >= size)
      return {};
    size_t end = range.get_end_offset();
    end = (end > size ? size : end);
    return { get_ptr() + begin, end - begin };
  }

  template<typename T, size_t max_size>
  template<typename T_, typename>
  constexpr bool StaticVector<T, max_size>::push_back(T&& to_move)
    noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    if (size == max_size)
      return false;
    new(get_ptr() + size++) T(std::move(to_move));
    return true;
  }

  template<typename T, size_t max_size>
  template<typename ...Args>
  constexpr bool StaticVector<T, max_size>::push_back(traits::InPlaceT, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>)
  {
    if (size == max_size)
      return false;
    new(get_ptr() + size++) T(std::forward<Args>(args)...);
    return true;
  }

  template<typename T>
  /// @brief Hash overload for Vector
  /// @tparam T The type of the Vector
  struct hash<Vector<T>>
  {
    /// @brief Hashing operator
    /// @param view The view to hash
    /// @return Hash
    constexpr size_t operator()(const Vector<T>& view) const noexcept
    {
      return GetHash(view.to_view());
    }
  };

  template<typename T>
  /// @brief Hash overload for StaticVector
  /// @tparam T The type of the StaticVector
  struct hash<SmallVector<T>>
  {
    /// @brief Hashing operator
    /// @param view The view to hash
    /// @return Hash
    constexpr size_t operator()(const StaticVector<T>& view) const noexcept
    {
      return GetHash(view.to_view());
    }
  };

  template<typename T, size_t sz>
  /// @brief Hash overload for StaticVector
  /// @tparam T The type of the StaticVector
  struct hash<StaticVector<T, sz>>
  {
    /// @brief Hashing operator
    /// @param view The view to hash
    /// @return Hash
    constexpr size_t operator()(const StaticVector<T, sz>& view) const noexcept
    {
      return GetHash(view.to_view());
    }
  };

#ifdef COLT_USE_IOSTREAMS
  template<typename T>
  static std::ostream& operator<<(std::ostream& os, const Vector<T>& var)
  {
    static_assert(traits::is_coutable_v<T>, "Type of Vector should implement operator<<(std::ostream&)!");
    os << ContiguousView<T>(var);
    return os;
  }

  template<typename T, size_t buff>
  static std::ostream& operator<<(std::ostream& os, const SmallVector<T, buff>& var)
  {
    static_assert(traits::is_coutable_v<T>, "Type of SmallVector should implement operator<<(std::ostream&)!");
    os << ContiguousView<T>(var);
    return os;
  }

  template<typename T, size_t buff>
  static std::ostream& operator<<(std::ostream& os, const StaticVector<T, buff>& var)
  {
    static_assert(traits::is_coutable_v<T>, "Type of StaticVector should implement operator<<(std::ostream&)!");
    os << ContiguousView<T>(var);
    return os;
  }
#endif  
}

#endif //!HG_COLT_VECTOR