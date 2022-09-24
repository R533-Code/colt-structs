/** @file allocator.h
* Allocator and memory helpers used throughout the front-end of the compiler.
* As C++'s allocators are magical, and only returns a pointer without size informations,
* the allocators in this file are not compatible with the STL's.
* All the allocators in this file work with MemBlock: a pointer and a size.
* The allocate/deallocate and new_t/delete_t functions interact with the global allocator.
* The global allocator is under a global lock, which means that allocate/deallocate are
* thread safe.
* Most allocators are taken from Andrei Alexandrescu's Memory Allocation talk:
* https://www.youtube.com/watch?v=LIb3L4vKZ7U
*/

#ifndef HG_ALLOCATOR
#define HG_ALLOCATOR

#include "common.h"

namespace colt
{
  /// @brief Contains memory allocation helpers.
  namespace memory
  {
    /********** MEMORY BLOCKS **********/

    /// @brief A MemBlock is the result of any allocation.
    /// An empty MemBlock is one whose 'ptr == nullptr'. The size
    /// of an empty MemBlock is usually the one of the allocation.
    class MemBlock
    {
      /// @brief Pointer to the beginning of the allocation
      void* ptr = nullptr;
      /// @brief Length (in bytes) of the allocation.
      /// Do not modify!
      size_t size = 0;

    public:
      /// @brief Constructs an empty MemBlock
      constexpr MemBlock() noexcept = default;

      /// @brief Constructs a MemBlock from a pointer and a size
      /// @param ptr The pointer to the memory block
      /// @param size The size of the memory block 
      constexpr MemBlock(void* ptr, size_t size) noexcept
        : ptr(ptr), size(size) {}

      /// @brief Check if the block is empty (getPtr() == nullptr)
      /// @return True if the block is empty
      constexpr bool isEmpty() const noexcept { return ptr == nullptr; }
      /// @brief Check if the block is empty (getPtr() != nullptr)
      /// @return True if the block is not empty
      constexpr bool isNotEmpty() const noexcept { return ptr != nullptr; }

      /// @brief Implicitly converts a block to a boolean, like a pointer
      /// @return True if the block is not empty
      constexpr explicit operator bool() const noexcept { return ptr != nullptr; }
      /// @brief Implicitly converts a block to a boolean, like a pointer
      /// @return True if the block is empty
      constexpr bool operator!() const noexcept { return ptr == nullptr; }

      /// @brief Get the pointer to the block
      /// @return Const pointer to the type of the block
      constexpr const void* getPtr() const noexcept { return ptr; }
      /// @brief Get the pointer to the block
      /// @return Pointer to the type of the block
      constexpr void* getPtr() noexcept { return ptr; }

      /// @brief Returns the byte size of the allocation
      /// @return The byte size of the allocation
      constexpr sizes::ByteSize getByteSize() const noexcept { return { size }; }

#ifdef COLT_DEBUG
      constexpr void*& impl_get_ptr() noexcept { return ptr; }
#endif
    };

    /// @brief A TypedBlock is the result of any typed allocation.
    /// An allocation results in a MemBlock, which holds a void*, and a byte size.
    /// To avoid "reinterpret_casting" at each access, and dividing byte size by
    /// sizeof(T), a TypedBlock holds a T* and a count of objects (not byte size).
    /// A TypedBlock does not call any constructor or destructor, it does not even
    /// begin the lifetime of an object. Use placement new in the TypedBlock to
    /// begin a lifetime and call explicitly the destructor.
    /// An empty TypedBlock is one whose 'ptr == nullptr'.
    template<typename T>
    class TypedBlock
    {
      static_assert(!std::is_array_v<T>, "A TypedBlock cannot be of an array type!");

      /// @brief Pointer to the beginning of the allocation
      T* ptr = nullptr;
      /// @brief Count of objects in the allocation.
      /// Do not modify!
      size_t size = 0;

    public:
      /// @brief Constructs an empty TypedBlock
      constexpr TypedBlock() noexcept = default;

      /// @brief Constructs a TypedBlock over a pointer and a size
      /// @param ptr The pointer to the memory block
      /// @param size The size of the memory block pointed by 'ptr'
      constexpr TypedBlock(void* ptr, size_t size) noexcept
        : ptr(std::launder(reinterpret_cast<T*>(ptr))), size(size / sizeof(T)) {}

      /// @brief Constructs a TypedBlock over a MemBlock
      /// @param blk The MemBlock
      constexpr TypedBlock(MemBlock blk) noexcept
        : ptr(std::launder(reinterpret_cast<T*>(blk.getPtr()))), size(blk.getByteSize().size / sizeof(T)) {}

      /// @brief Check if the block is empty (getPtr() == nullptr)
      /// @return True if the block is empty
      constexpr bool isEmpty() const noexcept { return ptr == nullptr; }
      /// @brief Check if the block is empty (getPtr() != nullptr)
      /// @return True if the block is not empty
      constexpr bool isNotEmpty() const noexcept { return ptr != nullptr; }

      /// @brief Implicitly converts a block to a boolean, like a pointer
      /// @return True if the block is not empty
      constexpr explicit operator bool() const noexcept { return ptr != nullptr; }
      /// @brief Implicitly converts a block to a boolean, like a pointer
      /// @return True if the block is empty
      constexpr bool operator!() const noexcept { return ptr == nullptr; }

      /// @brief Dereferences the pointer to the memory block
      /// @return Const reference to the type of the block
      constexpr const T& operator*() const noexcept { return *ptr; }
      /// @brief Dereferences the pointer to the memory block
      /// @return Reference to the type of the block
      constexpr T& operator*() noexcept { return *ptr; }

      /// @brief Dereferences the pointer to the memory block
      /// @return Const reference to the type of the block
      constexpr const T& operator->() const noexcept { return *ptr; }
      /// @brief Dereferences the pointer to the memory block
      /// @return Reference to the type of the block
      constexpr T& operator->() noexcept { return *ptr; }

      /// @brief Get the pointer to the block
      /// @return Const pointer to the type of the block
      constexpr const T* getPtr() const noexcept { return ptr; }
      /// @brief Get the pointer to the block
      /// @return Pointer to the type of the block
      constexpr T* getPtr() noexcept { return ptr; }

      /// @brief Returns the count of objects in the block
      /// @return The count of objects
      constexpr size_t getSize() const noexcept { return size; }
      /// @brief Returns the byte size of the allocation
      /// @return The byte size of the allocation
      constexpr sizes::ByteSize getByteSize() const noexcept { return { size * sizeof(T) }; }

      /// @brief Convert a TypedBlock to a MemBlock
      /// @return MemBlock
      constexpr operator MemBlock() const noexcept { return MemBlock{ reinterpret_cast<void*>(ptr), size * sizeof(T) }; }

#ifdef COLT_DEBUG
      constexpr T*& impl_get_ptr() noexcept { return ptr; }
#endif
    };

    /********** ALLOCATORS **********/

    /// @brief Allocator that always return NULL, and expects NULL
    class NULLAllocator
    {
    public:
      /// @brief Returns an empty MemBlock
      /// @param size Unused parameter
      /// @return Empty MemBlock
      MemBlock allocate(sizes::ByteSize size) noexcept { return { nullptr, 0 }; };

      /// @brief Ensures that 'to_free' is an empty MemBlock
      /// @param to_free The MemBlock to check for
      void deallocate(MemBlock to_free) noexcept { assert(to_free.getPtr() == nullptr && "ptr should be NULL!"); };
    };

    /// @brief Allocator that uses the heap, calling to malloc/free
    class Mallocator
    {
    public:
      /// @brief Allocates a MemBlock using the heap
      /// @param size The size of the allocation
      /// @return Allocated MemBlock or an empty MemBlock on failure
      MemBlock allocate(sizes::ByteSize size) noexcept
      {
        return { std::malloc(size.size), size.size };
      }

      /// @brief Deallocates a MemBlock that was allocated using 'Mallocator::allocate'
      /// @param to_free The block to free
      void deallocate(MemBlock to_free) noexcept
      {
        std::free(to_free.getPtr());
      }
    };

    template<size_t size>
    /// @brief Allocator that uses the stack for allocations
    class StackAllocator
    {
      /// @brief Stack Buffer used for allocation
      char buffer[size];
      /// @brief Pointer to where to allocate next
      char* top = buffer;

    public:
      /// @brief Allocates a MemBlock
      /// @param size The size of the allocation
      /// @return Allocated MemBlock or an empty MemBlock on failure
      MemBlock allocate(sizes::ByteSize size) noexcept;

      /// @brief Deallocates a MemBlock that was allocated using the current allocator
      /// @param to_free The block whose resources to free
      void deallocate(MemBlock to_free) noexcept;

      /// @brief Check if the current allocator owns 'blk'
      /// @param blk The MemBlock to check
      /// @return True if 'blk' was allocated through the current allocator
      bool owns(MemBlock blk) noexcept;

    private:
      /// @brief Round a size to the nearest aligned memory address
      /// @param sz The size to align
      /// @return Aligned size
      size_t align_up(size_t sz) noexcept;

      /// @brief Check if the stack has enough capacity to allocate 'sz'
      /// @param sz The aligned size to check
      /// @return True if the stack has enough capacity
      bool can_allocate(size_t sz) noexcept;

      /// @brief Check if the stack is empty
      /// @return 
      bool is_empty() noexcept;
    };

    template<typename allocator, size_t range_lower, size_t range_upper>
    /// @brief Allocator that reuses memory that is about to be cleared
    class FreeList
      : private allocator
    {
      static_assert(traits::is_allocator_v<allocator>, "'allocator' should be an allocator!");

      /// @brief A Node contains a pointer to the next Node
      struct Node
      {
        Node* next;
      };

      /// @brief Node to return on allocation
      Node* root = nullptr;

      /// @brief Check if a size is in range
      /// @param n The size to check
      /// @return True if range_lower <= n && n <= range_upper
      bool is_in_range(size_t n) noexcept { return range_lower <= n && n <= range_upper; }

    public:
      /// @brief Allocates a MemBlock
      /// @param size The size of the allocation
      /// @return Allocated MemBlock or an empty MemBlock on failure
      MemBlock allocate(sizes::ByteSize n) noexcept
      {
        if (root != nullptr)
        {
          if (is_in_range(n.size))
          {
            //Return the currently stored node
            MemBlock blk = { root, n.size };
            root = root->next;
            return blk;
          }
        }
        return allocator::allocate(n);
      }

      /// @brief Deallocates a MemBlock that was allocated using the current allocator
      /// @param to_free The block whose resources to free
      void deallocate(MemBlock blk) noexcept
      {
        if (!is_in_range(blk.getByteSize().size))
        {
          allocator::deallocate(blk);
          return;
        }
        
        //Add to memory to the free list
        auto p = reinterpret_cast<Node*>(blk.getPtr());
        p->next = root;
        root = p;
      }

      /// @brief Returns all the block owned by the FreeList to the underlying allocator
      ~FreeList() noexcept
      {
        while (root != nullptr)
        {
          auto next = root->next;
          allocator::deallocate({ reinterpret_cast<void*>(root), range_upper });
          root = next;
        }
      }
    };

    template<typename allocator>
    /// @brief Adds thread safety to any allocator using a mutex
    class ThreadSafeAllocator
      : public allocator
    {
      static_assert(traits::is_allocator_v<allocator>, "'allocator' should be an allocator!");

      /// @brief The mutex to ensure consistency across threads
      std::mutex mtx;

    public:
      /// @brief Allocates a MemBlock through the inherited allocator
      /// @param size The size of the allocation
      /// @return Allocated MemBlock or empty MemBlock
      MemBlock allocate(sizes::ByteSize size) noexcept { std::lock_guard<std::mutex> lock(mtx); return allocator::allocate(size); }

      /// @brief Deallocates a MemBlock that was allocated using the current allocator
      /// @param to_free The block whose resources to free
      void deallocate(MemBlock to_free) noexcept { std::lock_guard<std::mutex> lock(mtx); return allocator::deallocate(to_free); }

      /// @brief Check if the current allocator owns 'blk'
      /// @param blk The MemBlock to check
      /// @return True if 'blk' was allocated through the current allocator
      bool owns(MemBlock blk) noexcept { std::lock_guard<std::mutex> lock(mtx); return allocator::owns(blk); }
    };

    template<typename Primary, typename Fallback>
    /// @brief Allocator that tries to allocate through the Primary allocator and on failure uses the Fallback allocator
    class FallbackAllocator
      : private Primary, private Fallback
    {
      static_assert(traits::is_owning_allocator_v<Primary>, "'Primary' should be an owning allocator!");
      static_assert(traits::is_allocator_v<Fallback>, "'Fallback' should be an allocator!");

    public:
      /// @brief Allocates a MemBlock
      /// @param size The size of the allocation
      /// @return Allocated MemBlock or empty MemBlock
      MemBlock allocate(sizes::ByteSize size) noexcept;
      
      /// @brief Deallocates a MemBlock that was allocated using the current allocator
      /// @param to_free The block whose resources to free
      void deallocate(MemBlock to_free) noexcept;

      /// @brief Check if Primary or Fallback owns 'blk'
      /// @param blk The MemBlock to check
      /// @return True if 'blk' was allocated through the current allocator
      bool owns(MemBlock blk) noexcept;
    };

    template<size_t size, typename Primary, typename Secondary>
    /// @brief For all allocation sizes <= size, allocates through Primary, else through Secondary
    class Segregator
      : private Primary, private Secondary
    {
      static_assert(traits::is_allocator_v<Primary>, "'Primary' should be an owning allocator!");
      static_assert(traits::is_allocator_v<Secondary>, "'Secondary' should be an allocator!");

    public:
      /// @brief Allocates a MemBlock
      /// @param size The size of the allocation
      /// @return Allocated MemBlock or empty MemBlock
      MemBlock allocate(sizes::ByteSize size) noexcept;
    
      /// @brief Deallocates a MemBlock that was allocated using the current allocator
      /// @param to_free The block whose resources to free
      void deallocate(MemBlock to_free) noexcept;

      /// @brief Check if the current allocator owns 'blk'
      /// @param blk The MemBlock to check
      /// @return True if 'blk' was allocated through the current allocator
      bool owns(MemBlock blk) noexcept;
    };

    template<typename allocator, size_t register_size = 5>
    class AbortOnNULLAllocator
      : private allocator
    {
      static_assert(traits::is_allocator_v<allocator>, "'allocator' should be an allocator!");

      /// @brief The function pointer type that can be registered
      using register_fn_t = void(*)(void) noexcept;

      /// @brief The array of registered function pointer
      register_fn_t reg_array[register_size] = {};
      /// @brief The number of registered function.
      /// An atomic is used to protect the 'reg_array' from multiple threads accesses
      std::atomic<size_t> register_count;

    public:
      /// @brief Register function to call on exit
      /// @param func The function to register
      /// @return True if registering was successful, false if there is no more capacity for registering
      bool registerOnNullFn(void(*func)(void) noexcept) noexcept
      {
        if (register_count.load(std::memory_order_acquire) < register_size)
        {
          reg_array[register_count.fetch_add(1, std::memory_order_release)] = func;
          return true;
        }
        return false;
      }

      /// @brief Allocates a MemBlock through allocator
      /// @param size The size of the allocation
      /// @return Allocated MemBlock or empty MemBlock
      MemBlock allocate(sizes::ByteSize size) noexcept
      { 
        if (auto blk = allocator::allocate(size))
          return blk;
        //Call registered functions
        const size_t registered_count = register_count.load(std::memory_order_acquire);
        for (size_t i = 0; i < registered_count; i++)
          reg_array[i]();
        std::abort();
      }

      /// @brief Deallocates a MemBlock that was allocated using the current allocator
      /// @param to_free The block whose resources to free
      void deallocate(MemBlock to_free) noexcept { allocator::deallocate(to_free); };

      /// @brief Check if the current allocator owns 'blk'
      /// @param blk The MemBlock to check
      /// @return True if 'blk' was allocated through the current allocator
      bool owns(MemBlock blk) noexcept { return allocator::owns(blk); };
    };

    /************* PREDEFINED ALLOCATORS *************/

    /// @brief Allocator best suited for object of size smaller then 512
    using SmallAllocator_t =
      Segregator<256,
        FallbackAllocator<StackAllocator<8192>, FreeList<Mallocator, 0, 256>>,
        FreeList<Mallocator, 256, 512>
      >;

    /// @brief Global allocator type.
    /// Accesses to this allocator type are thread-safe.
    /// This allocator cannot return an empty block (nullptr), it will instead
    /// call std::abort(). To register a function to be called in that case,
    /// use RegisterOnNullFn(), which can register up to 5 functions (by default).
    using GlobalAllocator_t =
      AbortOnNULLAllocator<
      ThreadSafeAllocator<
        Segregator<512,
        SmallAllocator_t,
        FreeList<Mallocator, 512, 1024>>
      >>;
    
    /************* GLOBAL ALLOCATOR *************/
    
    namespace details
    {
      /// @brief Global allocator
      inline GlobalAllocator_t global_allocator;
    }

    /// @brief Reference to the global allocator.
    /// Accesses to the global allocator are thread-safe.
    GlobalAllocator_t& global_allocator = details::global_allocator;

    /// @brief Allocates a block of memory through the global allocator
    /// @param size The size of the block
    /// @return The non-null memory block
    inline MemBlock allocate(sizes::ByteSize size) noexcept
    {
      assert(size.size != 0 && "Cannot allocate 0 bytes!");
      return global_allocator.allocate(size);
    }

    /// @brief Deallocates a block of memory that was obtained through the global allocator
    /// @param blk The block to deallocate
    inline void deallocate(MemBlock blk) noexcept
    {
      global_allocator.deallocate(blk);
    }
    
    /// @brief Register a null callback for the global allocator.
    /// As the global allocator will exit instead of returning a nullptr,
    /// the user might want to print a message.
    /// @param fn The function pointer to register
    /// @return True if registering was successful, false if there is no more capacity for registering
    inline bool RegisterOnNullFn(void(*fn)(void) noexcept) noexcept
    {
      return global_allocator.registerOnNullFn(fn);
    }

    namespace details
    {
      template<bool is_noexcept, typename T, typename... Args>
      struct new_t_impl {};

      template<typename T, typename... Args>
      struct new_t_impl<true, T, Args...>
      {
        static inline TypedBlock<T> new_t(Args&&... args) noexcept
        {
          auto blk = allocate({ sizeof(T) });
          new(blk.getPtr()) T(std::forward<Args>(args)...);
          return blk;
        }
      };
      
      template<typename T, typename... Args>
      struct new_t_impl<false, T, Args...>
      {
        static inline TypedBlock<T> new_t(Args&&... args)
        {
          auto blk = allocate({ sizeof(T) });
          try
          {
            new(blk.getPtr()) T(std::forward<Args>(args)...);
            return blk;
          }
          catch (...) //avoid memory leak by freeing memory and re-throw
          {
            deallocate(blk);
            throw;
          }
        }
      };
      
      template<bool is_noexcept, typename T, typename... Args>
      struct delete_t_impl {};

      template<typename T, typename... Args>
      struct delete_t_impl<true, T, Args...>
      {
        static inline void delete_t(MemBlock COLT_REF_ON_DEBUG blk) noexcept
        {
          reinterpret_cast<T*>(blk.getPtr())->~T();
          deallocate(blk);
          //On debug, make the block empty to ensure that the user
          //does not use a deleted block
          COLT_ON_DEBUG(blk.impl_get_ptr() = nullptr);
        }
      };

      template<typename T, typename... Args>
      struct delete_t_impl<false, T, Args...>
      {
        static inline void delete_t(MemBlock COLT_REF_ON_DEBUG blk) noexcept
        {
          try
          {
            reinterpret_cast<T*>(blk.getPtr())->~T();
            deallocate(blk);
            //On debug, make the block empty to ensure that the user
            //does not use a deleted block
            COLT_ON_DEBUG(blk.impl_get_ptr() = nullptr);
          }
          catch (...)
          {
            deallocate(blk);
            COLT_ON_DEBUG(blk.impl_get_ptr() = nullptr);
            throw;
          }
        }
      };
    }

    template<typename T, typename... Args>
    /// @brief Constructs an object using the global allocator.
    /// No memory leaks can happen through this function: if the constructor throws,
    /// the memory is freed before propagating the exception.
    /// @tparam T The type to create
    /// @tparam ...Args The parameter pack
    /// @param ...args The argument pack to forward to the constructor
    /// @return The created object
    inline TypedBlock<T> new_t(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
      static_assert(!std::is_array_v<T>, "Use new_array_t for array types!");
      return 
        details::new_t_impl<std::is_nothrow_constructible_v<T, Args...>, T, Args...>::new_t(
          std::forward<Args>(args)...
        );
    }

    template<typename T>
    /// @brief Destroys and frees an object that was allocated through the global allocator
    /// @tparam T The type to destroy
    /// @param blk The block to destroy
    inline void delete_t(MemBlock COLT_REF_ON_DEBUG blk) noexcept(std::is_nothrow_destructible_v<T>)
    {
      static_assert(!std::is_array_v<T>, "Use delete_array_t for array types!");
      return details::delete_t_impl<std::is_nothrow_destructible_v<T>, T>::delete_t(blk);
    }

    template<typename T>
    /// @brief Destroys and frees an object that was allocated through the global allocator
    /// @tparam T The type to destroy
    /// @param blk The block to destroy
    inline void delete_t(TypedBlock<T> COLT_REF_ON_DEBUG blk) noexcept(std::is_nothrow_destructible_v<T>)
    {
      static_assert(!std::is_array_v<T>, "Use delete_array_t for array types!");
      return details::delete_t_impl<std::is_nothrow_destructible_v<T>, T>::delete_t(blk);
    }

    /************* FALLBACK ALLOCATOR *************/

    template<typename Primary, typename Fallback>
    MemBlock FallbackAllocator<Primary, Fallback>::allocate(sizes::ByteSize size) noexcept
    {
      if (MemBlock alloc = Primary::allocate(size))
        return alloc;
      return Fallback::allocate(size);
    }

    template<typename Primary, typename Fallback>
    void FallbackAllocator<Primary, Fallback>::deallocate(MemBlock to_free) noexcept
    {
      if (Primary::owns(to_free))
        Primary::deallocate(to_free);
      else
        Fallback::deallocate(to_free);
    }

    template<typename Primary, typename Fallback>
    bool FallbackAllocator<Primary, Fallback>::owns(MemBlock blk) noexcept
    {
      return Primary::owns(blk) || Fallback::owns(blk);
    }

    /************* STACK ALLOCATOR *************/

    template<size_t bsize>
    MemBlock StackAllocator<bsize>::allocate(sizes::ByteSize size) noexcept
    {
      size_t aligned_size = align_up(size.size);
      if (!can_allocate(aligned_size))
        return { nullptr, size.size };
      auto ptr = top;
      top += aligned_size;
      return { ptr, size.size };
    }

    template<size_t size>
    void StackAllocator<size>::deallocate(MemBlock to_free) noexcept
    {
      assert(!is_empty() && "StackAllocator was empty!");
      assert(owns(to_free) && "Block was not owned by the allocator!");

      size_t aligned_size = align_up(to_free.getByteSize().size);
      if (reinterpret_cast<char*>(to_free.getPtr()) + aligned_size == top)
        top -= aligned_size;
    }

    template<size_t size>
    bool StackAllocator<size>::owns(MemBlock blk) noexcept
    {
      return buffer <= blk.getPtr() && blk.getPtr() < top;
    }

    template<size_t size>
    size_t StackAllocator<size>::align_up(size_t sz) noexcept
    {
      //Do no round as already rounded
      if ((sz % alignof(std::max_align_t)) == 0)
        return sz;
      //Round size upward if needed
      return sz + (alignof(std::max_align_t) - (sz % alignof(std::max_align_t)));
    }

    template<size_t size>
    bool StackAllocator<size>::can_allocate(size_t sz) noexcept
    {
      return top + sz <= buffer + size;
    }

    template<size_t size>
    bool StackAllocator<size>::is_empty() noexcept
    {
      return top == buffer;
    }

    /************* SEGREGATOR ALLOCATOR *************/

    template<size_t bsize, typename Primary, typename Secondary>
    MemBlock Segregator<bsize, Primary, Secondary>::allocate(sizes::ByteSize size) noexcept
    {
      if (size.size <= bsize)
        return Primary::allocate(size);
      return Secondary::allocate(size);
    }

    template<size_t size, typename Primary, typename Secondary>
    void Segregator<size, Primary, Secondary>::deallocate(MemBlock to_free) noexcept
    {
      if (to_free.getByteSize().size <= size)
        Primary::deallocate(to_free);
      else
        Secondary::deallocate(to_free);
    }

    template<size_t size, typename Primary, typename Secondary>
    bool Segregator<size, Primary, Secondary>::owns(MemBlock blk) noexcept
    {
      return Primary::owns(blk) || Secondary::owns(blk);
    }
  }
}

#endif