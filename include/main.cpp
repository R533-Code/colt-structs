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
#include <colt/Iterators.h>
#include <colt/Enum.h>

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