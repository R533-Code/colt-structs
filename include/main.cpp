#include <cstdlib>

#define COLT_USE_IOSTREAMS
#include <colt/View.h>
#include <colt/Vector.h>
#include <colt/UniquePtr.h>
#include <colt/String.h>
#include <colt/Optional.h>
#include <colt/Expected.h>
#include <colt/Map.h>

using namespace colt;

int main()
{
  Map<char, uint64_t> map;
  map.insert('a', 0);
  map.insert('b', 1);
  map.insert('c', 2);
  if (auto a = map.find('a'))
    std::cout << std::boolalpha << "found a " << a->second << '\n';
  map.erase('a');
  map.insert('a', 0);
  if (map.contains('a'))
    std::cout << "Contains 'a'" << '\n';
  if (map.contains('b'))
    std::cout << "Contains 'b'" << '\n';

  for (auto& i : map)
  {
    i.second = 100;
  }
  std::cout << map;

  auto [ptr, iresult] = map.insert_or_assign('b', 1000);
  if (iresult == InsertionResult::ASSIGNED)
    std::cout << "b now has a value of " << ptr->second;
  else
    std::cout << ptr->second;
}