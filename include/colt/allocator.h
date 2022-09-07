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
		/// An empty MemBlock is one whose 'ptr = nullptr' and 'size = 0'.
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
			/// @return 
			constexpr MemBlock(void* ptr, size_t size) noexcept
				: ptr(ptr), size(size) {}

			constexpr const void* getPtr() const noexcept { return ptr; }
			constexpr void* getPtr() noexcept { return ptr; }

			constexpr sizes::ByteSize getSize() const noexcept { return size; }
			constexpr sizes::ByteSize getSize() noexcept { return size; }
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
			void deallocate(MemBlock to_free) noexcept { assert(to_free.ptr == nullptr && "ptr should be NULL!"); };
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
				auto mptr = std::malloc(size.size);
				if (mptr != nullptr)
					return { mptr, size };
				return { mptr, 0 };
			}

			/// @brief Deallocates a MemBlock that was allocated using 'Mallocator::allocate'
			/// @param to_free The block to free
			void deallocate(MemBlock to_free) noexcept
			{
				std::free(to_free.ptr);
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

		template<typename Alloc, size_t range_lower, size_t range_upper>
		/// @brief Allocator that reuses memory that is about to be cleared
		class FreeList
			: private Alloc
		{
			static_assert(traits::is_allocator_v<Alloc>, "'Alloc' should be an allocator!");

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
				return Alloc::allocate(n.size);
			}

			/// @brief Deallocates a MemBlock that was allocated using the current allocator
			/// @param to_free The block whose resources to free
			void deallocate(MemBlock blk) noexcept
			{
				if (!is_in_range(blk.size))
				{
					Alloc::deallocate(blk);
					return;
				}
				
				//Add to memory to the free list
				auto p = reinterpret_cast<Node*>(blk.ptr);
				p->next = root;
				root = p;
			}

			/// @brief Returns all the block owned by the FreeList to the underlying allocator
			~FreeList() noexcept
			{
				while (root != nullptr)
				{
					auto next = root->next;
					Alloc::deallocate({ reinterpret_cast<void*>(root), range_upper });
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
			MemBlock allocate(sizes::ByteSize size);
			
			/// @brief Deallocates a MemBlock that was allocated using the current allocator
			/// @param to_free The block whose resources to free
			void deallocate(MemBlock to_free);

			/// @brief Check if Primary or Fallback owns 'blk'
			/// @param blk The MemBlock to check
			/// @return True if 'blk' was allocated through the current allocator
			bool owns(MemBlock blk);
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
			MemBlock allocate(sizes::ByteSize size);
		
			/// @brief Deallocates a MemBlock that was allocated using the current allocator
			/// @param to_free The block whose resources to free
			void deallocate(MemBlock to_free);

			/// @brief Check if the current allocator owns 'blk'
			/// @param blk The MemBlock to check
			/// @return True if 'blk' was allocated through the current allocator
			bool owns(MemBlock blk);
		};

		/************* PREDEFINED ALLOCATORS *************/

		/// @brief Allocator best suited for object of size smaller then 512
		using SmallAllocator_t =
			Segregator<256,
				FallbackAllocator<StackAllocator<8192>, FreeList<Mallocator, 0, 256>>,
				FreeList<Mallocator, 256, 512>
			>;

		/// @brief Global allocator type
		using GlobalAllocator_t =
			ThreadSafeAllocator<
				Segregator<512,
				SmallAllocator_t,
				FreeList<Mallocator, 512, 1024>>
			>;
		
		/************* FALLBACK ALLOCATOR *************/

		template<typename Primary, typename Fallback>
		MemBlock FallbackAllocator<Primary, Fallback>::allocate(sizes::ByteSize size)
		{
			if (MemBlock alloc = Primary::allocate(size.size); alloc.ptr)
				return alloc;
			return Fallback::allocate(size);
		}

		template<typename Primary, typename Fallback>
		void FallbackAllocator<Primary, Fallback>::deallocate(MemBlock to_free)
		{
			if (Primary::owns(to_free))
				Primary::deallocate(to_free);
			else
				Fallback::deallocate(to_free);
		}

		template<typename Primary, typename Fallback>
		bool FallbackAllocator<Primary, Fallback>::owns(MemBlock blk)
		{
			return Primary::owns(blk) || Fallback::owns(blk);
		}

		/************* STACK ALLOCATOR *************/

		template<size_t bsize>
		MemBlock StackAllocator<bsize>::allocate(sizes::ByteSize size) noexcept
		{
			size_t aligned_size = align_up(size.size);
			if (!can_allocate(aligned_size))
				return { nullptr, 0 };
			auto ptr = top;
			top += aligned_size;
			return { ptr, size.size };
		}

		template<size_t size>
		void StackAllocator<size>::deallocate(MemBlock to_free) noexcept
		{
			assert(!is_empty() && "StackAllocator was empty!");
			assert(owns(to_free) && "Block was not owned by the allocator!");

			size_t aligned_size = align_up(to_free.size);
			if (reinterpret_cast<char*>(to_free.ptr) + aligned_size == top)
				top -= aligned_size;
		}

		template<size_t size>
		bool StackAllocator<size>::owns(MemBlock blk) noexcept
		{
			return buffer <= blk.ptr && blk.ptr < top;
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
		MemBlock Segregator<bsize, Primary, Secondary>::allocate(sizes::ByteSize size)
		{
			if (size.size <= bsize)
				return Primary::allocate(size);
			return Secondary::allocate(size);
		}

		template<size_t size, typename Primary, typename Secondary>
		void Segregator<size, Primary, Secondary>::deallocate(MemBlock to_free)
		{
			if (to_free.size <= size)
				Primary::deallocate(to_free);
			else
				Secondary::deallocate(to_free);
		}

		template<size_t size, typename Primary, typename Secondary>
		bool Segregator<size, Primary, Secondary>::owns(MemBlock blk)
		{
			return Primary::owns(blk) || Secondary::owns(blk);
		}
	}
}

#endif