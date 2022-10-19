#include <cstdlib>

#define COLT_USE_IOSTREAMS
#include <colt/View.h>
#include <colt/Vector.h>
#include <colt/UniquePtr.h>
#include <colt/String.h>
#include <colt/Optional.h>
#include <colt/Expected.h>
#include <colt/Map.h>
#include <colt/List.h>
#include <colt/Set.h>

using namespace colt;

int main()
{
  StableSet<String> hello;
  auto a = hello.insert(String{ "Hello" });
  if (a.first == hello.insert(String{ "Hello" }).first)
    std::cout << "Working!";
  else
    std::cout << "Not Working!";
  std::cout << '\n' << hello;
}