/** @file UniquePtr.h
* Contains a UniquePtr class that uses the allocators in 'memory/allocator.h'.
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
  /// This avoids memory leaks that could happen if a UniquePtr of type child,
  /// is converted to UniquePtr of type parent (and sizeof(child) > sizeof(parent).
  /// This is why the releaseTyped() can only be used when hintIsTrueType() is true.
  /// @tparam T The type pointed to by the UniquePtr
  class UniquePtr
  {
    template<typename Ty>
    friend class UniquePtr;

    /// @brief The MemBlock owned by the UniquePtr
    memory::MemBlock blk = { nullptr, 0 };

  public:
    // No copy constructor
    UniquePtr(const UniquePtr&) = delete;
    // No copy assignment operator
    UniquePtr& operator=(const UniquePtr&) = delete;

    /// @brief Default constructs an empty UniquePtr
    constexpr UniquePtr() noexcept = default;
    /// @brief Default constructs an empty UniquePtr
    /// @param  nullptr_t
    constexpr UniquePtr(std::nullptr_t) noexcept {}

    /// @brief Constructs a UniquePtr from a TypedBlock
    /// @param blk The block whose ownership to steal
    constexpr UniquePtr(memory::TypedBlock<T> blk) noexcept
      : blk(blk) {}
    
    /// @brief Constructs a UniquePtr from a MemBlock
    /// @param blk The block whose ownership to steal
    constexpr UniquePtr(memory::MemBlock blk) noexcept
      : blk(blk) {}

    /// @brief Move constructor
    /// @param to_move The UniquePtr whose resources to steal
    constexpr UniquePtr(UniquePtr&& to_move) noexcept
      : blk(exchange(to_move.blk, { nullptr, 0 })) {}
    /// @brief Move assignment operator
    /// @param to_move The UniquePtr whose resources to steal
    /// @return Self
    constexpr UniquePtr& operator=(UniquePtr&& to_move) noexcept
    {
      swap(to_move.blk, blk);
      return *this;
    }
    
    template<typename T2>
    /// @brief Move constructor for inheritances
    /// @tparam T2 The type of the block whose ownership to steal
    /// @param blk The block whose ownership to steal
    constexpr UniquePtr(std::enable_if_t<std::is_convertible_v<T*, T2*>, memory::TypedBlock<T2>> blk) noexcept
      : blk(blk) {}
    
    template<typename T2>
    /// @brief Move constructor for inheritances
    /// @tparam T2 The type of the UniquePtr whose resources to steal
    /// @param to_move The UniquePtr whose resources to steal
    constexpr UniquePtr(std::enable_if_t<std::is_convertible_v<T*, T2*>, UniquePtr<T2>> to_move) noexcept
      : blk(blk) {}
    
    template<typename T2>
    /// @brief Move assignment operator for inheritances
    /// @tparam T2 The type of the UniquePtr whose resources to steal
    /// @param to_move The UniquePtr whose resources to steal
    /// @return Self
    constexpr UniquePtr& operator=(std::enable_if_t<std::is_convertible_v<T*, T2*>, UniquePtr<T2>>&& to_move) noexcept
    {
      swap(to_move.blk, blk);
      return *this;
    }

    /// @brief Destructor, delete the owned resource
    ~UniquePtr() noexcept(std::is_nothrow_destructible_v<T>)
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

    /// @brief Check if the current block has the size of the UniquePtr type.
    /// This is a hint: in inheritance, a UniquePtr of child class can be assigned
    /// to a UniquePtr of base class. If both do not have the same size, this information
    /// is retained through the owned block's byte size.
    /// @return True if getByteSize() == sizeof(T)
    constexpr bool hintIsTrueType() const noexcept { return blk.getByteSize() == sizeof(T); }
    /// @brief Check if the current block has the size of the UniquePtr type.
    /// This is a hint: in inheritance, a UniquePtr of child class can be assigned
    /// to a UniquePtr of base class. If both do not have the same size, this information
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

  template<typename PtrT>
  /// @brief A UniquePtr is a relocatable resource
  /// @tparam PtrT The type of the UniquePtr
  struct traits::is_relocatable<UniquePtr<PtrT>>
  {
    static constexpr bool value = true;
  };

  template<typename T, typename... Args>
  /// @brief Creates a UniquePtr of type T pointing to a T constructed with 'args'
  /// @tparam T The type to construct
  /// @tparam ...Args The parameter pack
  /// @param ...args The argument pack
  /// @return UniquePtr of type T
  UniquePtr<T> make_unique(Args&&... args) noexcept(std::is_nothrow_constructible_v<T>)
  {
    return memory::new_t<T>(std::forward<Args>(args)...);
  }

#ifdef COLT_USE_IOSTREAMS
  template<typename T>
  static std::ostream& operator<<(std::ostream& os, const UniquePtr<T>& var)
  {
    static_assert(traits::is_coutable_v<T>, "Type of UniquePtr should implement operator<<(std::ostream&)!");
    os << var.getPtr();
    return os;
  }
#endif
}

#endif //!HG_UNIQUE_PTR