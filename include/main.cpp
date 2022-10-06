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

using namespace colt;

int main()
{
  FlatList<int, 2> hello;
  hello.push_back(1);
  hello.push_back(10);
  hello.push_back(100);
  hello.push_back(1000);

  auto end = hello.end();
  for (auto it = hello.begin(); it != end; ++it)
  {
    *it = 0;
  }

  std::cout << hello;
}