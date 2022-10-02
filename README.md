# Colt struct:
Contains various data structures, and C++ utilities.

# Allocators:
This library implements various allocators with the purpose of making allocations faster.
Allocations result in a `MemBlock` or a pointer and a size.
Allocations through the global allocator of the library cannot fail, which means that if an allocation will return an empty `MemBlock`, the allocator will call functions registered with `RegisterOnNULLFn` followed by an `abort`.

# Data Structures:
- `Optional`: Optional value that can contain a value.
- `Expected`: Expected value that can contain a value or an error value.

All the allocations that these structures perform uses the Colt global allocator.
- `Vector`: Contiguous dynamic array of objects.
- `SmallVector`: Contiguous dynamic array of objects with a stack buffer.
- `String`: Optionally NUL terminated dynamic array of characters with a stack buffer.
- `UniquePtr`: Automatically managed pointer to a resource
- `Map`: Key/Value associative container