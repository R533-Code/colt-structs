#ifndef HG_COLT_VECTOR
#define HG_COLT_VECTOR

#include <initializer_list>

#include "View.h"
#include "details/allocator.h"
#include "details/algorithm.h"

namespace colt
{
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
    constexpr Vector(size_t fill_size, traits::InPlaceT, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>);      

    /// @brief Constructs a Vector from an initializer list.
    /// Only works for copyable T.
    /// @param list The list from which to copy the object
    constexpr Vector(std::initializer_list<T> list) noexcept(std::is_nothrow_copy_constructible_v<T>);

    /// @brief Constructs a Vector from a view.
    /// @param view The view whose items to copy
    constexpr Vector(ContiguousView<T> view) noexcept(std::is_nothrow_copy_constructible_v<T>);      

    /// @brief Copy constructor.
    /// Only works for copyable T.
    /// @param to_copy Vector whose resources to copy
    constexpr Vector(const Vector& to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>);

    /// @brief Copy assignment operator.
    /// Only works for copyable T.
    /// Precondition: &to_copy != this (no self assignment)
    /// @param to_copy Vector whose resources to copy
    /// @return Self
    constexpr Vector& operator=(const Vector& to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>);    

    /// @brief Move constructor
    /// @param to_move Vector whose resources to steal
    constexpr Vector(Vector&& to_move) noexcept;      

    /// @brief Move assignment operator.
    /// Precondition: &to_move != this (no self assignment)
    /// @param to_move Vector whose resources to steal
    /// @return Self
    constexpr Vector& operator=(Vector&& to_move) noexcept;    

    /// @brief Destructor
    ~Vector() noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Returns an iterator to the beginning of the Vector
    /// @return Iterator to the beginning
    constexpr ContiguousIterator<T> begin() noexcept { return blk.getPtr(); }
    /// @brief Returns a iterator to the end of the Vector
    /// @return Iterator to the end
    constexpr ContiguousIterator<T> end() noexcept { return blk.getPtr() + size; }

    /// @brief Returns a const iterator to the beginning of the Vector
    /// @return Const iterator to the beginning
    constexpr ContiguousIterator<const T> begin() const noexcept { return blk.getPtr(); }
    /// @brief Returns a const iterator to the end of the Vector
    /// @return Const iterator to the end
    constexpr ContiguousIterator<const T> end() const noexcept { return blk.getPtr() + size; }

    /// @brief Returns a pointer to the beginning of the data
    /// @return Const pointer to the beginning of the data
    constexpr const T* getData() const noexcept { return blk.getPtr(); }
    /// @brief Returns a pointer to the beginning of the data
    /// @return Pointer to the beginning of the data
    constexpr T* getData() noexcept { return blk.getPtr(); }

    /// @brief Returns the count of active objects in the Vector
    /// @return The count of objects in the Vector
    constexpr size_t getSize() const noexcept { return size; }
    /// @brief Returns the capacity of the current allocation
    /// @return The capacity of the current allocation
    constexpr size_t getCapacity() const noexcept { return blk.getSize(); }
    
    /// @brief Returns the byte size of the allocation
    /// @return ByteSize of the allocation
    constexpr sizes::ByteSize getByteSize() const noexcept { return blk.getByteSize(); }

    /// @brief Returns the object at index 'index' of the Vector.
    /// Precondition: index < size
    /// @param index The index of the object
    /// @return The object at index 'index'
    constexpr traits::copy_if_trivial_t<const T&> operator[](size_t index) const noexcept;    

    /// @brief Returns a reference to the object at index 'index' of the Vector.
    /// Precondition: index < size
    /// @param index The index of the object
    /// @return The object at index 'index'
    constexpr T& operator[](size_t index) noexcept;

    /// @brief Check if the Vector does not contain any object.
    /// Same as: getSize() == 0
    /// @return True if the Vector is empty
    constexpr bool isEmpty() const noexcept { return size == 0; }

    /// @brief Check if the Vector does not contain any object.
    /// Same as: getSize() != 0
    /// @return True if the Vector is not empty
    constexpr bool isNotEmpty() const noexcept { return size != 0; }

    /// @brief Reserve 'by_more' object
    /// @param by_more The count of object to reserve for
    constexpr void reserve(size_t by_more) noexcept(std::is_trivially_copyable_v<T> || std::is_nothrow_move_constructible_v<T>);

    /// @brief Push an object at the end of the Vector by copying
    /// @param to_copy The object to copy at the end of the Vector
    constexpr void pushBack(traits::copy_if_trivial_t<const T&> to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>);

    template<typename T_ = T, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    /// @brief Push an object at the end of the Vector by moving
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param to_move The object to move at the end of the Vector
    constexpr void pushBack(T&& to_move) noexcept(std::is_nothrow_move_constructible_v<T>);

    template<typename... Args>
    /// @brief Emplace an object at the end of the Vector
    /// @tparam ...Args The parameter pack
    /// @param  InPlaceT tag
    /// @param ...args The argument pack to forward to the constructor
    constexpr void pushBack(traits::InPlaceT, Args&&... args) noexcept(std::is_constructible_v<T, Args...>);

    /// @brief Pops an item from the back of the Vector.
    /// Precondition: isNotEmpty()
    constexpr void popBack() noexcept(std::is_nothrow_destructible_v<T>);

    /// @brief Pops N item from the back of the Vector.
    /// Precondition: N <= getSize()
    /// @param N The number of item to pop from the back
    constexpr void popBackN(size_t N) noexcept(std::is_nothrow_destructible_v<T>);

    /// @brief Removes all the item from the Vector.
    /// This does not modify the capacity of the Vector.
    constexpr void clear() noexcept(std::is_nothrow_destructible_v<T>);

    /// @brief Returns the first item in the Vector.
    /// Precondition: !isEmpty()
    /// @return The first item in the Vector.
    constexpr traits::copy_if_trivial_t<const T&> getFront() const noexcept;
    /// @brief Returns the first item in the Vector.
    /// Precondition: !isEmpty()
    /// @return The first item in the Vector.
    constexpr T& getFront() noexcept;

    /// @brief Returns the last item in the Vector.
    /// Precondition: !isEmpty()
    /// @return The last item in the Vector.
    constexpr traits::copy_if_trivial_t<const T&> getBack() const noexcept;
    /// @brief Returns the last item in the Vector.
    /// Precondition: !isEmpty()
    /// @return The last item in the Vector.
    constexpr T& getBack() noexcept;

    /// @brief Obtains a view over the whole Vector
    /// @return View over the Vector
    constexpr ContiguousView<T> toView() const noexcept { return { blk.getPtr(), size }; }

    /// @brief Obtains a view over the 'range' of the Vector.
    /// @param range The range to obtain from the Vector
    /// @return View over 'range' of the Vector
    constexpr ContiguousView<T> toView(Range range) const noexcept;

    /// @brief Converts a Vector to a view implicitly
    /// @return ContiguousView over the whole Vector
    constexpr explicit operator ContiguousView<T>() const noexcept { return { blk.getPtr(), size }; }
  };

  template<typename T, size_t buff_count = 5>
  /// @brief A Vector with a small buffer optimization.
  /// A SmallVector is a vector containing an internal stack buffer
  /// that can contain up to 'buff_count' elements.
  /// This allows to avoid memory allocations for small counts of elements.
  /// This can be a pessimization if the SmallVector regularly contains more
  /// than 'buff_count' elements: the internal stack buffer occupies memory
  /// that will not be used.
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

  public:
    /// @brief Default constructor
    constexpr SmallVector() noexcept = default;

    /// @brief Creates a SmallVector with a capacity of 'reserve_size'.
    /// If 'reserve_size' is smaller than 'buff_count', then the capacity
    /// is set to 'buff_count'
    /// @param reserve_size The capacity of the SmallVector
    constexpr SmallVector(size_t reserve_size) noexcept;      

    /// @brief Copy constructor
    /// @param to_copy The SmallVector whose data to copy
    constexpr SmallVector(const SmallVector& to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>);

    /// @brief Move constructor
    /// @param to_move The SmallVector whose data to move
    constexpr SmallVector(SmallVector&& to_move) noexcept(std::is_nothrow_move_constructible_v<T> || std::is_trivially_copyable_v<T>);

    /// @brief Destructor
    ~SmallVector() noexcept(std::is_nothrow_destructible_v<T>);    

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

    constexpr const T* getData() const noexcept { return get_current_ptr(); }
    constexpr T* getData() noexcept { return get_current_ptr(); }

    /// @brief Check if the SmallVector is empty
    /// @return True if empty
    constexpr bool isEmpty() const noexcept { return size == 0; }
    /// @brief Check if the SmallVector is not empty
    /// @return True if not empty
    constexpr bool isNotEmpty() const noexcept { return size != 0; }

    /// @brief The count of active elements in the SmallVector
    /// @return The count of active elements in the SmallVector
    constexpr size_t getSize() const noexcept { return size; }
    /// @brief Returns the capacity of the current allocation.
    /// If the SmallVector is currently using stack memory, returns 'buff_count'
    /// @return Capacity of the current allocation or 'buff_count'
    constexpr size_t getCapacity() const noexcept { return capacity; }

    /// @brief Check if the SmallVector is using its internal stack-allocated buffer
    /// @return True if the elements are stored in the stack-allocated buffer
    constexpr bool isStackAllocated() const noexcept { return capacity == buff_count; }

    /// @brief Returns the byte size of the allocation
    /// @return ByteSize of the allocation
    constexpr sizes::ByteSize getByteSize() const noexcept { return { capacity * sizeof(T) }; }

    /// @brief Returns the object at index 'index' of the Vector.
    /// Precondition: index < size
    /// @param index The index of the object
    /// @return The object at index 'index'
    constexpr traits::copy_if_trivial_t<const T&> operator[](size_t index) const noexcept;

    /// @brief Returns a reference to the object at index 'index' of the Vector.
    /// Precondition: index < size
    /// @param index The index of the object
    /// @return The object at index 'index'
    constexpr T& operator[](size_t index) noexcept;

    /// @brief Push an object at the end of the Vector by copying
    /// @param to_copy The object to copy at the end of the Vector
    constexpr void pushBack(traits::copy_if_trivial_t<const T&> to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>);

    template<typename T_ = T, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    /// @brief Push an object at the end of the Vector by moving
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param to_move The object to move at the end of the Vector
    constexpr void pushBack(T&& to_move) noexcept(std::is_nothrow_move_constructible_v<T>);    

    template<typename... Args>
    /// @brief Emplace an object at the end of the Vector
    /// @tparam ...Args The parameter pack
    /// @param  InPlaceT tag
    /// @param ...args The argument pack to forward to the constructor
    constexpr void pushBack(traits::InPlaceT, Args&&... args) noexcept(std::is_constructible_v<T, Args...>);    

    /// @brief Pops an item from the back of the Vector.
    /// Precondition: isNotEmpty()
    constexpr void popBack() noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Pops N item from the back of the Vector.
    /// Precondition: N <= getSize()
    /// @param N The number of item to pop from the back
    constexpr void popBackN(size_t N) noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Removes all the item from the Vector.
    /// This does not modify the capacity of the Vector.
    constexpr void clear() noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Returns the first item in the Vector.
    /// Precondition: !isEmpty()
    /// @return The first item in the Vector.
    constexpr traits::copy_if_trivial_t<const T&> getFront() const noexcept;    
    /// @brief Returns the first item in the Vector.
    /// Precondition: !isEmpty()
    /// @return The first item in the Vector.
    constexpr T& getFront() noexcept;    

    /// @brief Returns the last item in the Vector.
    /// Precondition: !isEmpty()
    /// @return The last item in the Vector.
    constexpr traits::copy_if_trivial_t<const T&> getBack() const noexcept;    
    /// @brief Returns the last item in the Vector.
    /// Precondition: !isEmpty()
    /// @return The last item in the Vector.
    constexpr T& getBack() noexcept;    

    /// @brief Reserves 'by_more' capacity.
    /// This always causes an allocation, so be sure that the stack capacity is not
    /// enough before calling.
    /// @param by_more The count of object to reserve for
    constexpr void reserve(size_t by_more) noexcept(std::is_nothrow_move_constructible_v<T> || std::is_trivially_copyable_v<T>);

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
  };

  template<typename T>
  constexpr Vector<T>::Vector(size_t reserve) noexcept
    : blk(memory::allocate({ reserve * sizeof(T) })), size(0) {}

  template<typename T>
  constexpr Vector<T>::Vector(std::initializer_list<T> list) noexcept(std::is_nothrow_copy_constructible_v<T>)
    : blk(memory::allocate({ list.size() * sizeof(T) })), size(list.size())
  {
    algo::contiguous_copy(list.begin(), blk.getPtr(), size);
  }

  template<typename T>
  constexpr Vector<T>::Vector(ContiguousView<T> view) noexcept(std::is_nothrow_copy_constructible_v<T>)
    : blk(memory::allocate({ view.getSize() * sizeof(T) })), size(view.getSize())
  {
    algo::contiguous_copy(list.begin(), blk.getPtr(), size);
  }

  template<typename T>
  constexpr Vector<T>::Vector(const Vector& to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>)
    : blk(memory::allocate(to_copy.blk.getByteSize())), size(to_copy.getSize())
  {
    algo::contiguous_copy(list.begin(), blk.getPtr(), size);
  }

  template<typename T>
  constexpr Vector<T>& Vector<T>::operator=(const Vector& to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>)
  {
    assert(&to_copy != this && "Self assignment is prohibited!");

    clear();
    for (size_t i = 0; i < to_copy.size; i++)
      pushBack(to_copy[i]);

    return *this;
  }

  template<typename T>
  constexpr Vector<T>::Vector(Vector&& to_move) noexcept
    : blk(exchange(to_move.blk, { nullptr, 0 })), size(exchange(to_move.size, 0)) {}

  template<typename T>
  constexpr Vector<T>& Vector<T>::operator=(Vector&& to_move) noexcept
  {
    assert(&to_move != this && "Self assignment is prohibited!");

    //Swap members
    swap(to_move.blk, blk);
    swap(to_move.size, size);

    return *this;
  }

  template<typename T>
  Vector<T>::~Vector() noexcept(std::is_nothrow_destructible_v<T>)
  {
    clear();
    if (blk)
      memory::deallocate(blk);
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> Vector<T>::operator[](size_t index) const noexcept
  {
    assert(index < size && "Invalid index!");
    return blk.getPtr()[index];
  }

  template<typename T>
  constexpr T& Vector<T>::operator[](size_t index) noexcept
  {
    assert(index < size && "Invalid index!");
    return blk.getPtr()[index];
  }

  template<typename T>
  constexpr void Vector<T>::reserve(size_t by_more) noexcept(std::is_trivially_copyable_v<T> || std::is_nothrow_move_constructible_v<T>)
  {
    memory::TypedBlock<T> new_blk = memory::allocate({ blk.getByteSize().size + by_more * sizeof(T) });
    
    algo::contiguous_destructive_move(blk.getPtr(), new_blk.getPtr(), size);

    memory::deallocate(blk);
    blk = new_blk;
  }

  template<typename T>
  constexpr void Vector<T>::pushBack(traits::copy_if_trivial_t<const T&> to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>)
  {
    if (size == blk.getSize())
      reserve(blk.getSize() + 4);
    new(blk.getPtr() + size) T(to_copy);
    ++size;
  }

  template<typename T>
  constexpr void Vector<T>::popBack() noexcept(std::is_nothrow_destructible_v<T>)
  {
    assert(!isEmpty() && "Vector was empty!");
    --size;
    blk.getPtr()[size].~T();
  }

  template<typename T>
  constexpr void Vector<T>::popBackN(size_t N) noexcept(std::is_nothrow_destructible_v<T>)
  {
    assert(N <= size && "Vector does not contain enough items!");
    for (size_t i = size - N; i < size; i++)
      blk.getPtr()[i].~T();
    size -= N;
  }

  template<typename T>
  constexpr void Vector<T>::clear() noexcept(std::is_nothrow_destructible_v<T>)
  {
    algo::contiguous_destruct(blk.getPtr(), size);
    size = 0;
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> Vector<T>::getFront() const noexcept
  {
    assert(!isEmpty() && "Vector was empty!");
    return *blk.getPtr();
  }

  template<typename T>
  constexpr T& Vector<T>::getFront() noexcept
  {
    assert(!isEmpty() && "Vector was empty!");
    return *blk.getPtr();
  }

  template<typename T>
  constexpr traits::copy_if_trivial_t<const T&> Vector<T>::getBack() const noexcept
  {
    assert(!isEmpty() && "Vector was empty!");
    return blk.getPtr()[size - 1];
  }

  template<typename T>
  constexpr T& Vector<T>::getBack() noexcept
  {
    assert(!isEmpty() && "Vector was empty!");
    return blk.getPtr()[size - 1];
  }

  template<typename T>
  constexpr ContiguousView<T> Vector<T>::toView(Range range) const noexcept
  {
    size_t begin = range.getBeginOffset();
    size_t end = range.getEndOffset();
    end = (end > size ? size : end);
    return { begin_ptr + begin, end - begin };
  }
  
  template<typename T>
  template<typename ...Args>
  constexpr Vector<T>::Vector(size_t fill_size, traits::InPlaceT, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args ...>)
    : blk(memory::allocate({ fill_size * sizeof(T) })), size(fill_size)
  {
    algo::contiguous_construct(blk.getPtr(), size, std::forward<Args>(args)...);
  }
  
  template<typename T>
  template<typename T_, typename>
  constexpr void Vector<T>::pushBack(T&& to_move) noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    if (size == blk.getSize())
      reserve(blk.getSize() + 4);
    new(blk.getPtr() + size) T(std::move(to_move));
    ++size;
  }
    
  template<typename T>
  template<typename ...Args>
  constexpr void Vector<T>::pushBack(traits::InPlaceT, Args&&... args) noexcept(std::is_constructible_v<T, Args ...>)
  {
    if (size == blk.getSize())
      reserve(blk.getSize() + 4);
    new(blk.getPtr() + size) T(std::forward<Args>(args)...);
    ++size;
  }

  template<typename T, size_t buff_count>
  constexpr SmallVector<T, buff_count>::SmallVector(size_t reserve_size) noexcept
    : capacity(reserve_size <= buff_count ? buff_count : reserve_size)
  {
    //make the allocation pointer active
    if (capacity == buff_count)
      ptr = reinterpret_cast<T*>(memory::allocate({ buff_count * sizeof(T) }).getPtr());
  }

  template<typename T, size_t buff_count>
  constexpr SmallVector<T, buff_count>::SmallVector(const SmallVector& to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>)
    : capacity(to_copy.capacity), size(to_copy.size)
  {
    if (!isStackAllocated())
    {
      ptr = reinterpret_cast<T*>(memory::allocate({ buff_count * sizeof(T) }).getPtr());
      algo::contiguous_copy(to_copy.ptr, ptr, size);
    }
    else
    {
      T* const to = get_stack_ptr();
      T* const from = to_move.get_stack_ptr();
      algo::contiguous_copy(from, to, size);
    }
  }

  template<typename T, size_t buff_count>
  constexpr SmallVector<T, buff_count>::SmallVector(SmallVector&& to_move) noexcept(std::is_nothrow_move_constructible_v<T> || std::is_trivially_copyable_v<T>)
    : capacity(to_move.capacity), size(exchange(to_move.size, 0))
  {
    if (!isStackAllocated())
    {
      ptr = exchange(to_move.ptr, nullptr);
    }
    else
    {
      T* const to = get_stack_ptr();
      T* const from = to_move.get_stack_ptr();
      algo::contiguous_move(from, to, size);
    }
  }

  template<typename T, size_t buff_count>
  SmallVector<T, buff_count>::~SmallVector() noexcept(std::is_nothrow_destructible_v<T>)
  {
    clear();
    if (!isStackAllocated())
      if (ptr)
        memory::deallocate({ ptr, capacity * sizeof(T) });
  }

  template<typename T, size_t buff_count>
  constexpr traits::copy_if_trivial_t<const T&> SmallVector<T, buff_count>::operator[](size_t index) const noexcept
  {
    assert(index < size && "Invalid index!");
    return get_current_ptr()[index];
  }

  template<typename T, size_t buff_count>
  constexpr T& SmallVector<T, buff_count>::operator[](size_t index) noexcept
  {
    assert(index < size && "Invalid index!");
    return get_current_ptr()[index];
  }

  template<typename T, size_t buff_count>
  constexpr void SmallVector<T, buff_count>::pushBack(traits::copy_if_trivial_t<const T&> to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>)
  {
    if (size == capacity)
      reserve(capacity);
    new(get_current_ptr() + size) T(to_copy);
    ++size;
  }

  template<typename T, size_t buff_count>
  constexpr void SmallVector<T, buff_count>::popBack() noexcept(std::is_nothrow_destructible_v<T>)
  {
    assert(!isEmpty() && "Vector was empty!");
    --size;
    get_current_ptr()[size].~T();
  }

  template<typename T, size_t buff_count>
  constexpr void SmallVector<T, buff_count>::popBackN(size_t N) noexcept(std::is_nothrow_destructible_v<T>)
  {
    assert(N <= size && "Vector does not contain enough items!");
    T* const ptr_d = get_current_ptr();
    algo::contiguous_destruct(ptr_d + (size -= N), N);
  }

  template<typename T, size_t buff_count>
  constexpr void SmallVector<T, buff_count>::clear() noexcept(std::is_nothrow_destructible_v<T>)
  {
    T* const ptr_d = get_current_ptr();
    algo::contiguous_destruct(ptr_d, size);
    size = 0;
  }

  template<typename T, size_t buff_count>
  constexpr traits::copy_if_trivial_t<const T&> SmallVector<T, buff_count>::getFront() const noexcept
  {
    assert(!isEmpty() && "Vector was empty!");
    return *get_current_ptr();
  }

  template<typename T, size_t buff_count>
  constexpr T& SmallVector<T, buff_count>::getFront() noexcept
  {
    assert(!isEmpty() && "Vector was empty!");
    return *get_current_ptr();
  }

  template<typename T, size_t buff_count>
  constexpr traits::copy_if_trivial_t<const T&> SmallVector<T, buff_count>::getBack() const noexcept
  {
    assert(!isEmpty() && "Vector was empty!");
    return get_current_ptr()[size - 1];
  }

  template<typename T, size_t buff_count>
  constexpr T& SmallVector<T, buff_count>::getBack() noexcept
  {
    assert(!isEmpty() && "Vector was empty!");
    return get_current_ptr()[size - 1];
  }

  template<typename T, size_t buff_count>
  constexpr void SmallVector<T, buff_count>::reserve(size_t by_more) noexcept(std::is_nothrow_move_constructible_v<T> || std::is_trivially_copyable_v<T>)
  {
    memory::TypedBlock<T> blk = memory::allocate({ sizeof(T) * (capacity + by_more) });
    T* const ptr_d = get_current_ptr();
    
    algo::contiguous_destructive_move(ptr_d, blk.getPtr(), size);
    
    if (!isStackAllocated())
      memory::deallocate({ ptr_d, capacity * sizeof(T) });
    capacity += by_more;
    ptr = blk.getPtr();
  }

  template<typename T, size_t buff_count>
  constexpr const T* SmallVector<T, buff_count>::get_current_ptr() const noexcept
  {
    if (isStackAllocated())
      return get_stack_ptr();
    return ptr;
  }

  template<typename T, size_t buff_count>
  constexpr T* SmallVector<T, buff_count>::get_current_ptr() noexcept
  {
    if (isStackAllocated())
      return get_stack_ptr();
    return ptr;
  }
  
  template<typename T, size_t buff_count>
  template<typename T_, typename>
  constexpr void SmallVector<T, buff_count>::pushBack(T&& to_move) noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    if (size == capacity)
      reserve(capacity);
    new(get_current_ptr() + size) T(std::move(to_move));
    ++size;
  }

  template<typename T, size_t buff_count>
  template<typename ...Args>
  constexpr void SmallVector<T, buff_count>::pushBack(traits::InPlaceT, Args && ...args) noexcept(std::is_constructible_v<T, Args ...>)
  {
    if (size == capacity)
      reserve(capacity);
    new(get_current_ptr() + size) T(std::forward<Args>(args)...);
    ++size;
  }

  template<typename T>
  std::size_t hash(const Vector<T>& view) noexcept
  {
    return get_hash(ContiguousView<T>(view));
  }

  template<typename T, size_t sz>
  std::size_t hash(const SmallVector<T, sz>& view) noexcept
  {
    return get_hash(ContiguousView<T>(view));
  }

#ifdef COLT_USE_IOSTREAMS
  template<typename T>
  static std::ostream& operator<<(std::ostream& os, const Vector<T>& var)
  {
    os << ContiguousView<T>(var);
    return os;
  }

  template<typename T, size_t buff>
  static std::ostream& operator<<(std::ostream& os, const SmallVector<T, buff>& var)
  {
    os << ContiguousView<T>(var);
    return os;
  }
#endif  
}

#endif //!HG_COLT_VECTOR