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
  while (!feof(stdin))
  {
    auto str = String::getLine();
    if (str.is_error())
      break;
    hello.insert(str.get_value());
  }
  std::cout << '\n' << hello;
}