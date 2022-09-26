//truetrue[0, 1, 2, 3, 4, 5]false[0, 1, 2, 3, 4, 5, 6][0, 1, 2, 3, 4, 5, 6][0, 1, 2, 3, 4, 5, 6]No leaks!
#include <cstdlib>

#define COLT_USE_IOSTREAMS
#include "colt/Vector.h"

static uint64_t alloc_count;
static uint64_t free_count;

struct Test
{
  int* ptr;

  Test()
    : ptr((int*)malloc(100))
  {
    alloc_count += 1;
  }

  Test(size_t hello)
    : ptr((int*)malloc(hello))
  {
    alloc_count += 1;
  }
  
  Test(Test&& ptra) noexcept
    : ptr(colt::exchange(ptra.ptr, nullptr))
  {}

  ~Test()
  {
    free(ptr);
    if (ptr)
      free_count += 1;
  }
};

using namespace colt;

int main(int argc, char** argv)
{
  {
    SmallVector<uint64_t, 6> vec1;
    if (vec1.isStackAllocated())
      fputs("true", stdout);
    else
      fputs("false", stdout);

    for (size_t i = 0; i < 6; i++)
      vec1.pushBack(i);
    if (vec1.isStackAllocated())
      fputs("true", stdout);
    else
      fputs("false", stdout);

    std::cout << vec1;
    vec1.pushBack(6);
    if (vec1.isStackAllocated())
      fputs("true", stdout);
    else
      fputs("false", stdout);
    
    SmallVector<uint64_t, 6> vec2(vec1);
    std::cout << vec1 << vec2;
    SmallVector<uint64_t, 6> vec3(std::move(vec2));
    std::cout << vec3;
  }
  {
    SmallVector<Test, 6> vec1;    

    for (size_t i = 0; i < 6; i++)
      vec1.pushBack(InPlace, i);
    vec1.pushBack(InPlace, 100);
  }
  if (alloc_count == free_count)
    fputs("No leaks!", stdout);
}