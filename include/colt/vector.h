#ifndef HG_COLT_VECTOR
#define HG_COLT_VECTOR

#include <initializer_list>

#include "view.h"
#include "allocator.h"

namespace colt
{
  template<typename T>
  class Vector
  {
    memory::TypedBlock<T> blk = { nullptr, 0 };
    size_t size = 0;

  public:
    constexpr Vector() noexcept = default;

    explicit Vector(size_t reserve) noexcept
      : blk(memory::allocate({ reserve * sizeof(T) })), size(0) {}

    ~Vector() noexcept(std::is_nothrow_destructible_v<T>)
    {
      for (size_t i = 0; i < size; i++)
        blk.getPtr()[i].~T();
      COLT_ON_DEBUG(size = 0);
      if (blk)
        memory::deallocate(blk);
    }

    constexpr const T* getData() const noexcept { return blk.getPtr(); }
    constexpr T* getData() noexcept { return blk.getPtr(); }

    constexpr size_t getSize() const noexcept { return size; }
    constexpr size_t getCapacity() const noexcept { return blk.getSize(); }
    
    constexpr sizes::ByteSize getByteSize() const noexcept { return blk.getByteSize(); }

    constexpr traits::copy_if_trivial_t<const T> operator[](size_t index) const noexcept
    {
      assert(index < size && "Invalid index!");
      return blk.getPtr()[index];
    }

    constexpr T& operator[](size_t index) noexcept
    {
      assert(index < size && "Invalid index!");
      return blk.getPtr()[index];
    }

    bool isEmpty() const noexcept { return size == 0; }

    bool isNotEmpty() const noexcept { return size != 0; }

    void reserve(size_t by_more) noexcept
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

    void pushBack(traits::copy_if_trivial_t<const T> to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
      if (size == blk.getSize())
        reserve(blk.getSize() + 4);
      new(blk.getPtr() + size) T(to_copy);
      ++size;
    }

    template<typename T_ = T, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    void pushBack(T&& to_move) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
      if (size == blk.getSize())
        reserve(blk.getSize() + 4);
      new(blk.getPtr() + size) T(std::move(to_move));
      ++size;
    }

    template<typename... Args>
    void pushBack(traits::InPlaceT, Args&&... args) noexcept(std::is_constructible_v<T, Args...>)
    {
      if (size == blk.getSize())
        reserve(blk.getSize() + 4);
      new(blk.getPtr() + size) T(std::forward<Args>(args)...);
      ++size;
    }

    void popBack() noexcept(std::is_nothrow_destructible_v<T>)
    {
      assert(!isNotEmpty() && "Vector was empty!");
      --size;
      blk.getPtr()[size].~T();
    }

    void popBackN(size_t N) noexcept(std::is_nothrow_destructible_v<T>)
    {
      assert(N <= size && "Vector does not contain enough items!");
      for (size_t i = size - N; i < size; i++)
        blk.getPtr()[i].~T();
      size -= N;
    }

    void clear() noexcept(std::is_nothrow_destructible_v<T>)
    {
      for (size_t i = 0; i < size; i++)
        blk.getPtr()[i].~T();
      size = 0;
    }

    constexpr explicit operator ContiguousView<T>() const noexcept { return { blk.getPtr(), size }; }
  };
}

#endif //!HG_COLT_VECTOR