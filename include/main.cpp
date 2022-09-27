#include <cstdlib>

#define COLT_USE_IOSTREAMS
#include <colt/View.h>
#include <colt/Vector.h>
#include <colt/UniquePtr.h>
#include <colt/String.h>
#include <colt/Optional.h>
#include <colt/Expected.h>
#include <colt/MultiMap.h>

using namespace colt;

int main()
{
  Map<char, uint64_t> map = { 100 };
  map.insert('a', 100);
  if (auto a = map.find('a'))
    std::cout << "found a " << a->second << '\n';
  auto [ptr, iresult] = map.insertOrAssign('b', 1000);
  if (iresult == InsertionResult::ASSIGNED)
    std::cout << "a now has a value of " << ptr->second;
  else
    std::cout << ptr->second;
}