#ifndef HG_COLT_VECTOR
#define HG_COLT_VECTOR

#include <initializer_list>

#include "view.h"
#include "allocator.h"

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
    explicit Vector(size_t reserve) noexcept;

    template<typename... Args>
    /// @brief Constructs and fills a Vector of 'fill_size' by forwarding 'args' to the constructor
    /// @tparam ...Args The parameter pack
    /// @param fill_size The count of object to reserve
    /// @param  InPlaceT tag
    /// @param ...args The argument pack
    Vector(size_t fill_size, traits::InPlaceT, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>);      

    /// @brief Constructs a Vector from an initializer list.
    /// Only works for copyable T.
    /// @param list The list from which to copy the object
    Vector(std::initializer_list<T> list) noexcept(std::is_nothrow_copy_constructible_v<T>);

    /// @brief Constructs a Vector from a view.
    /// @param view The view whose items to copy
    Vector(ContiguousView<T> view) noexcept(std::is_nothrow_copy_constructible_v<T>);      

    /// @brief Copy constructor.
    /// Only works for copyable T.
    /// @param to_copy Vector whose resources to copy
    Vector(const Vector& to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>);

    /// @brief Copy assignment operator.
    /// Only works for copyable T.
    /// Precondition: &to_copy != this (no self assignment)
    /// @param to_copy Vector whose resources to copy
    /// @return Self
    Vector& operator=(const Vector& to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>);    

    /// @brief Move constructor
    /// @param to_move Vector whose resources to steal
    Vector(Vector&& to_move) noexcept;      

    /// @brief Move assignment operator.
    /// Precondition: &to_move != this (no self assignment)
    /// @param to_move Vector whose resources to steal
    /// @return Self
    Vector& operator=(Vector&& to_move) noexcept;    

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
    constexpr traits::copy_if_trivial_t<const T> operator[](size_t index) const noexcept;    

    /// @brief Returns a reference to the object at index 'index' of the Vector.
    /// Precondition: index < size
    /// @param index The index of the object
    /// @return The object at index 'index'
    constexpr T& operator[](size_t index) noexcept;    

    /// @brief Check if the Vector does not contain any object.
    /// Same as: getSize() == 0
    /// @return True if the Vector is empty
    bool isEmpty() const noexcept { return size == 0; }

    /// @brief Check if the Vector does not contain any object.
    /// Same as: getSize() != 0
    /// @return True if the Vector is not empty
    bool isNotEmpty() const noexcept { return size != 0; }

    /// @brief Reserve 'by_more' object
    /// @param by_more The count of object to reserve for
    void reserve(size_t by_more) noexcept;    

    /// @brief Push an object at the end of the Vector by copying
    /// @param to_copy The object to copy at the end of the Vector
    void pushBack(traits::copy_if_trivial_t<const T> to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>);    

    template<typename T_ = T, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    /// @brief Push an object at the end of the Vector by moving
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param to_move The object to move at the end of the Vector
    void pushBack(T&& to_move) noexcept(std::is_nothrow_move_constructible_v<T>);    

    template<typename... Args>
    /// @brief Emplace an object at the end of the Vector
    /// @tparam ...Args The parameter pack
    /// @param  InPlaceT tag
    /// @param ...args The argument pack to forward to the constructor
    void pushBack(traits::InPlaceT, Args&&... args) noexcept(std::is_constructible_v<T, Args...>);    

    /// @brief Pops an item from the back of the Vector.
    /// Precondition: isNotEmpty()
    void popBack() noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Pops N item from the back of the Vector.
    /// Precondition: N <= getSize()
    /// @param N The number of item to pop from the back
    void popBackN(size_t N) noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Removes all the item from the Vector.
    /// This does not modify the capacity of the Vector.
    void clear() noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Returns the first item in the Vector.
    /// Precondition: !isEmpty()
    /// @return The first item in the Vector.
    traits::copy_if_trivial_t<const T> getFront() const noexcept;
    /// @brief Returns the first item in the Vector.
    /// Precondition: !isEmpty()
    /// @return The first item in the Vector.
    T& getFront() noexcept;

    /// @brief Returns the last item in the Vector.
    /// Precondition: !isEmpty()
    /// @return The last item in the Vector.
    traits::copy_if_trivial_t<const T> getBack() const noexcept;
    /// @brief Returns the last item in the Vector.
    /// Precondition: !isEmpty()
    /// @return The last item in the Vector.
    T& getBack() noexcept;

    /// @brief Obtains a view over the whole Vector
    /// @return View over the Vector
    constexpr ContiguousView<T> toView() const noexcept { return { blk.getPtr(), size }; }

    /// @brief Converts a Vector to a view implicitly
    /// @return ContiguousView over the whole Vector
    constexpr explicit operator ContiguousView<T>() const noexcept { return { blk.getPtr(), size }; }
  };

  template<typename T>
  Vector<T>::Vector(size_t reserve) noexcept
    : blk(memory::allocate({ reserve * sizeof(T) })), size(0) {}

  template<typename T>
  Vector<T>::Vector(std::initializer_list<T> list) noexcept(std::is_nothrow_copy_constructible_v<T>)
    : blk(memory::allocate({ list.size() * sizeof(T) })), size(list.size())
  {
    for (size_t i = 0; i < size; i++) //copy construct
      new(blk.getPtr() + i) T(std::data(list)[i]);
  }

  template<typename T>
  Vector<T>::Vector(ContiguousView<T> view) noexcept(std::is_nothrow_copy_constructible_v<T>)
    : blk(memory::allocate({ view.getSize() * sizeof(T) })), size(view.getSize())
  {
    for (size_t i = 0; i < size; i++) //copy construct
      new(blk.getPtr() + i) T(view.getData()[i]);
  }

  template<typename T>
  Vector<T>::Vector(const Vector& to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>)
    : blk(memory::allocate(to_copy.blk.getByteSize())), size(to_copy.getSize())
  {
    for (size_t i = 0; i < size; i++) //copy construct
      new(blk.getPtr() + i) T(to_copy.blk.getPtr()[i]);
  }

  template<typename T>
  Vector<T>& Vector<T>::operator=(const Vector& to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>)
  {
    assert(&to_copy != this && "Self assignment is prohibited!");

    clear();
    for (size_t i = 0; i < to_copy.size; i++)
      pushBack(to_copy[i]);

    return *this;
  }

  template<typename T>
  Vector<T>::Vector(Vector&& to_move) noexcept
    : blk(exchange(to_move.blk, { nullptr,0 })), size(exchange(to_move.size, 0)) {}

  template<typename T>
  Vector<T>& Vector<T>::operator=(Vector&& to_move) noexcept
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
  constexpr traits::copy_if_trivial_t<const T> Vector<T>::operator[](size_t index) const noexcept
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
  void Vector<T>::reserve(size_t by_more) noexcept
  {
    memory::TypedBlock<T> new_blk = memory::allocate({ blk.getByteSize().size + by_more * sizeof(T) });
    if constexpr (std::is_trivial_v<T>)
    {
      std::memcpy(new_blk.getPtr(), blk.getPtr(), size * sizeof(T));
    }
    else
    {
      for (size_t i = 0; i < size; i++)
        new(new_blk.getPtr() + i) T(std::move(blk.getPtr()[i]));
      clear();
    }
    memory::deallocate(blk);
    blk = new_blk;
  }

  template<typename T>
  void Vector<T>::pushBack(traits::copy_if_trivial_t<const T> to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>)
  {
    if (size == blk.getSize())
      reserve(blk.getSize() + 4);
    new(blk.getPtr() + size) T(to_copy);
    ++size;
  }

  template<typename T>
  void Vector<T>::popBack() noexcept(std::is_nothrow_destructible_v<T>)
  {
    assert(!isEmpty() && "Vector was empty!");
    --size;
    blk.getPtr()[size].~T();
  }

  template<typename T>
  void Vector<T>::popBackN(size_t N) noexcept(std::is_nothrow_destructible_v<T>)
  {
    assert(N <= size && "Vector does not contain enough items!");
    for (size_t i = size - N; i < size; i++)
      blk.getPtr()[i].~T();
    size -= N;
  }

  template<typename T>
  void Vector<T>::clear() noexcept(std::is_nothrow_destructible_v<T>)
  {
    for (size_t i = 0; i < size; i++)
      blk.getPtr()[i].~T();
    size = 0;
  }

  template<typename T>
  traits::copy_if_trivial_t<const T> Vector<T>::getFront() const noexcept
  {
    assert(!isEmpty() && "Vector was empty!");
    return *blk.getPtr();
  }

  template<typename T>
  T& Vector<T>::getFront() noexcept
  {
    assert(!isEmpty() && "Vector was empty!");
    return *blk.getPtr();
  }

  template<typename T>
  traits::copy_if_trivial_t<const T> Vector<T>::getBack() const noexcept
  {
    assert(!isEmpty() && "Vector was empty!");
    return blk.getPtr()[size - 1];
  }

  template<typename T>
  T& Vector<T>::getBack() noexcept
  {
    assert(!isEmpty() && "Vector was empty!");
    return blk.getPtr()[size - 1];
  }
  
  template<typename T>
  template<typename ...Args>
  Vector<T>::Vector(size_t fill_size, traits::InPlaceT, Args && ...args) noexcept(std::is_nothrow_constructible_v<T, Args ...>)
    : blk(memory::allocate({ fill_size * sizeof(T) })), size(fill_size)
  {
    for (size_t i = 0; i < size; i++)
      new(blk.getPtr() + i) T(std::forward<Args>(args)...)
  }
  
  template<typename T>
  template<typename T_, typename>
  void Vector<T>::pushBack(T&& to_move) noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    if (size == blk.getSize())
      reserve(blk.getSize() + 4);
    new(blk.getPtr() + size) T(std::move(to_move));
    ++size;
  }
    
  template<typename T>
  template<typename ...Args>
  void Vector<T>::pushBack(traits::InPlaceT, Args && ...args) noexcept(std::is_constructible_v<T, Args ...>)
  {
    if (size == blk.getSize())
      reserve(blk.getSize() + 4);
    new(blk.getPtr() + size) T(std::forward<Args>(args)...);
    ++size;
  }

#ifdef COLT_USE_IOSTREAMS
  template<typename T>
  static std::ostream& operator<<(std::ostream& os, const Vector<T>& var)
  {
    static_assert(traits::is_coutable_v<T>, "Type of Vector should implement operator<<(std::ostream&)!");
    os << '[';
    if (!var.isEmpty())
      os << var.getFront();
    for (size_t i = 1; i < var.getSize(); i++)
      os << ", " << var[i];
    os << ']';
    return os;
  }
#endif
}

#endif //!HG_COLT_VECTOR