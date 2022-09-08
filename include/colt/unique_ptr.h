/** @file unique_ptr.h
* Contains a unique_ptr class that uses the allocators in 'memory/allocator.h'.
*/

#ifndef HG_COLT_UNIQUE_PTR
#define HG_COLT_UNIQUE_PTR

#include "allocator.h"

namespace colt
{
  template<typename T>
  /// @brief Unique pointer that automatically frees an allocation resources.
  /// A unique pointer is movable but not copyable, which makes it own its resource.
  /// This unique pointer class deals internally with MemBlock, not TypedBlock:
  /// This avoids memory leaks that could happen if a unique_ptr of type child,
  /// is converted to unique_ptr of type parent (and sizeof(child) > sizeof(parent).
  /// This is why the releaseTyped() can only be used when hintIsTrueType() is true.
  /// @tparam T The type pointed to by the unique_ptr
  class unique_ptr
  {
    template<typename Ty>
    friend class unique_ptr;

    /// @brief The MemBlock owned by the unique_ptr
    memory::MemBlock blk = { nullptr, 0 };

  public:
    // No copy constructor
    unique_ptr(const unique_ptr&) = delete;
    // No copy assignment operator
    unique_ptr& operator=(const unique_ptr&) = delete;

    /// @brief Default constructs an empty unique_ptr
    constexpr unique_ptr() noexcept = default;
    /// @brief Default constructs an empty unique_ptr
    /// @param  nullptr_t
    constexpr unique_ptr(std::nullptr_t) noexcept {}

    /// @brief Constructs a unique_ptr from a TypedBlock
    /// @param blk The block whose ownership to steal
    constexpr unique_ptr(memory::TypedBlock<T> blk) noexcept
      : blk(blk) {}
    
    /// @brief Constructs a unique_ptr from a MemBlock
    /// @param blk The block whose ownership to steal
    constexpr unique_ptr(memory::MemBlock blk) noexcept
      : blk(blk) {}

    /// @brief Move constructor
    /// @param to_move The unique_ptr whose resources to steal
    constexpr unique_ptr(unique_ptr&& to_move) noexcept
      : blk(exchange(to_move.blk, { nullptr, 0 })) {}
    /// @brief Move assignment operator
    /// @param to_move The unique_ptr whose resources to steal
    /// @return Self
    constexpr unique_ptr& operator=(unique_ptr&& to_move) noexcept
    {
      swap(to_move.blk, blk);
      return *this;
    }
    
    template<typename T2>
    /// @brief Move constructor for inheritances
    /// @tparam T2 The type of the block whose ownership to steal
    /// @param blk The block whose ownership to steal
    constexpr unique_ptr(std::enable_if_t<std::is_convertible_v<T*, T2*>, memory::TypedBlock<T2>> blk) noexcept
      : blk(blk) {}
    
    template<typename T2>
    /// @brief Move constructor for inheritances
    /// @tparam T2 The type of the unique_ptr whose resources to steal
    /// @param to_move The unique_ptr whose resources to steal
    constexpr unique_ptr(std::enable_if_t<std::is_convertible_v<T*, T2*>, unique_ptr<T2>> to_move) noexcept
      : blk(blk) {}
    
    template<typename T2>
    /// @brief Move assignment operator for inheritances
    /// @tparam T2 The type of the unique_ptr whose resources to steal
    /// @param to_move The unique_ptr whose resources to steal
    /// @return Self
    constexpr unique_ptr& operator=(std::enable_if_t<std::is_convertible_v<T*, T2*>, unique_ptr<T2>>&& to_move) noexcept
    {
      swap(to_move.blk, blk);
      return *this;
    }

    /// @brief Destructor, delete the owned resource
    ~unique_ptr() noexcept(std::is_nothrow_destructible_v<T>)
    {
      if (blk)
        memory::delete_t<T>(blk);
    }

    /// @brief Implicitly converts a block to a boolean, like a pointer
      /// @return True if the block is not empty
    constexpr explicit operator bool() const noexcept { return blk.operator bool(); }
    /// @brief Implicitly converts a block to a boolean, like a pointer
    /// @return True if the block is empty
    constexpr bool operator!() const noexcept { return blk.operator!(); }

    /// @brief Dereferences the pointer to the memory block
    /// @return Const reference to the type of the block
    constexpr const T& operator*() const noexcept { return *reinterpret_cast<const T*>(blk.getPtr()); }
    /// @brief Dereferences the pointer to the memory block
    /// @return Reference to the type of the block
    constexpr T& operator*() noexcept { return *reinterpret_cast<T*>(blk.getPtr()); }

    /// @brief Dereferences the pointer to the memory block
    /// @return Const reference to the type of the block
    constexpr const T* operator->() const noexcept { return reinterpret_cast<const T*>(blk.getPtr()); }
    /// @brief Dereferences the pointer to the memory block
    /// @return Reference to the type of the block
    constexpr T* operator->() noexcept { return reinterpret_cast<T*>(blk.getPtr()); }

    /// @brief Check if the owned block is empty (getPtr() == nullptr)
    /// @return True if the block is empty
    constexpr bool isNull() const noexcept { return blk.getPtr() == nullptr; }
    /// @brief Check if the owned block is empty (getPtr() != nullptr)
    /// @return True if the block is not empty
    constexpr bool isNotNull() const noexcept { return blk.getPtr() != nullptr; }    

    /// @brief Get the pointer to the block
      /// @return Const pointer to the type of the block
    constexpr const T* getPtr() const noexcept { return reinterpret_cast<const T*>(blk.getPtr()); }
    /// @brief Get the pointer to the block
    /// @return Pointer to the type of the block
    constexpr T* getPtr() noexcept { return reinterpret_cast<T*>(blk.getPtr()); }

    /// @brief Check if the current block has the size of the unique_ptr type.
    /// This is a hint: in inheritance, a unique_ptr of child class can be assigned
    /// to a unique_ptr of base class. If both do not have the same size, this information
    /// is retained through the owned block's byte size.
    /// @return True if getByteSize() == sizeof(T)
    constexpr bool hintIsTrueType() const noexcept { return blk.getByteSize() == sizeof(T); }
    /// @brief Check if the current block has the size of the unique_ptr type.
    /// This is a hint: in inheritance, a unique_ptr of child class can be assigned
    /// to a unique_ptr of base class. If both do not have the same size, this information
    /// is retained through the owned block's byte size.
    /// @return True if getByteSize() != sizeof(T)
    constexpr bool hintIsNotTrueType() const noexcept { return blk.getByteSize() != sizeof(T); }

    /// @brief Releases ownership of the owned TypedBlock.
    /// Precondition: hintTrueType() == true.
    /// This method should not be used when inheritances are used.
    /// @return The owned TypedBlock
    constexpr memory::TypedBlock<T> releaseTyped() noexcept
    {
      assert(hintIsTrueType() && "Use release() instead when dealing with inheritances!");
      return exchange(blk, { nullptr, 0 });
    }

    /// @brief Releases ownership of the owned MemBlock.
    /// This method should be used when inheritances are used.
    /// @return The owned MemBlock
    constexpr memory::MemBlock release() noexcept
    {      
      return exchange(blk, { nullptr, 0 });
    }

    /// @brief Returns the byte size of the allocation
    /// @return The byte size of the allocation
    constexpr sizes::ByteSize getByteSize() const noexcept { return blk.getByteSize(); }    
  };

  template<typename T, typename... Args>
  unique_ptr<T> make_unique(Args&&... args) noexcept(std::is_nothrow_constructible_v<T>)
  {
    return memory::new_t<T>(std::forward<Args>(args)...);
  }

#ifdef COLT_USE_IOSTREAMS
  template<typename T>
  static std::ostream& operator<<(std::ostream& os, const unique_ptr<T>& var)
  {
    static_assert(traits::is_coutable_v<T>, "Type of unique_ptr should implement operator<<(std::ostream&)!");
    os << var.getPtr();
    return os;
  }
#endif
}

#endif //!HG_UNIQUE_PTR