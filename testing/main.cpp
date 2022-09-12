#include <cstdio>

#define COLT_USE_IOSTREAMS
#define COLT_USE_FMT
#include <colt/allocator.h>
#include <colt/common.h>
#include <colt/view.h>
#include <colt/optional.h>
#include <colt/unique_ptr.h>
#include <colt/vector.h>
#include <colt/expected.h>

//TODO: add {fmt} formatting by checking for FMT_VERSION

using namespace colt;

struct Test
{
  Test() { fputs("Constructed!\n", stdout); }
  Test(Test&&) noexcept { fputs("Move-Constructed!\n", stdout); }
  Test(const Test&) noexcept { fputs("Copy-Constructed!\n", stdout); }
  ~Test() { fputs("Destructed!\n", stdout); }
};

using namespace colt::sizes;

void print_no_memory() noexcept
{
  fputs("Not enough memory to perform allocation!", stdout);
}

int main()
{
  memory::registerOnNullFn(print_no_memory);
}