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

 #define OS_ENUM(XX) XX(Windows, 10)XX(Linux, 30)XX(MacOs, 32)XX(Android, 40)
 
DECLARE_VALUE_ENUM(OsEnum, uint8_t, OS_ENUM);

int main()
{
	String a = String{ "Hello world!" };
	for (auto i : a.to_iter() | iter::adapt)
		std::cout << i << ' ';
}