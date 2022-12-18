#include <cstdlib>

#define COLT_USE_IOSTREAMS
#include <colt/data_structs/View.h>
#include <colt/data_structs/Vector.h>
#include <colt/data_structs/UniquePtr.h>
#include <colt/data_structs/String.h>
#include <colt/data_structs/Optional.h>
#include <colt/data_structs/Expected.h>
#include <colt/data_structs/Map.h>
#include <colt/data_structs/List.h>
#include <colt/data_structs/Set.h>
#include <colt/utility/Iterators.h>
#include <colt/refl/Enum.h>

using namespace colt;
using namespace colt::refl;

#define OS_ENUM(XX) XX(Windows, 10)XX(Linux, 30)XX(MacOs, 32)XX(Android, 40)

DECLARE_VALUE_ENUM(OsEnum, uint8_t, OS_ENUM);

template<typename... Args>
void print(Args&&... args) noexcept
{
  for_each(
    [](auto&& a)
    {
      using t = std::decay_t<decltype(a)>;

      //Print type name 
      std::cout << info<t>::name << ": "
        << std::forward<decltype(a)>(a) << '\n';
    },
    std::forward<Args>(args)...);
}

struct Point
{
  u32 x;
  u32 y;
  u32 z;
  PTR<u32> a = &z;
};

#define POINT_MEMBERS(XX) XX(Point::x)XX(Point::y)XX(Point::z)XX(Point::a)
DECLARE_TYPE(Point, POINT_MEMBERS);

int main()
{
  Point a = { 10, 20, 30 };
  print(a);

  std::cout << "----------------------------\n";
  print(10, 20, 1.0, "Hello world!");
  std::cout << "----------------------------\n";

  for (auto i : info<Point>::to_member_str_iter() | iter::adapt)
    std::cout << i << "\n";
}