cmake_minimum_required(VERSION 3.4)
# C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ColtStructs VERSION 0.0.0.0 LANGUAGES CXX)

# Contains all headers in 'include/colt'
file(GLOB ColtHeaders "include/colt/*.h")

add_executable(colt_test
	${ColtHeaders}
	"testing/main.cpp"
)

target_include_directories(colt_test PUBLIC
	"include"
)