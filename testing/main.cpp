#include <cstdio>
#include <colt/common.h>
#include <colt/view.h>
#include <colt/optional.h>

using namespace colt;

struct Test
{
  Test() { fputs("Constructed!\n", stdout); }
  Test(Test&&) { fputs("Move-Constructed!\n", stdout); }
  Test(const Test&) { fputs("Move-Constructed!\n", stdout); }
  ~Test() { fputs("Destructed!\n", stdout); }
};

Optional<int> getInt()
{
  return 10;
}

int main()
{
  auto a = getInt();
  auto b = a;
  if (a)
    printf("a: has value %d\n", a.getValue());
  else
    printf("None\n");
}