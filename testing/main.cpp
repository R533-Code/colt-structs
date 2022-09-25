#include <cstdio>

#define COLT_USE_IOSTREAMS
#define COLT_USE_FMT
#include <colt/View.h>
#include <colt/Optional.h>
#include <colt/UniquePtr.h>
#include <colt/Vector.h>
#include <colt/Expected.h>
#include <colt/String.h>
#include <colt/Hash.h>

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
  memory::RegisterOnNULLFn(print_no_memory);
}