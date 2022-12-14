cmake_minimum_required(VERSION 3.4)
# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ColtStructs VERSION 0.0.3.0 LANGUAGES CXX)

# Contains all headers in 'include/colt'
file(GLOB_RECURSE ColtHeaders "include/colt/*.h")

add_executable(colt_test
	${ColtHeaders}
	"include/main.cpp"
)

target_include_directories(colt_test PUBLIC
	"include"
)

message(STATUS "Searching for tests...")
# Load tests
file(GLOB_RECURSE ColtTestsPath "testing/*.cpp")
list(LENGTH ColtTestsPath testCount)
message(STATUS "Found " ${testCount} " tests.")

# Testing for CTest
enable_testing()

# TESTING:
# Each file in resources/tests/ that ends with .ct
# is a file to try to run.
# The first line of these files is a comment '//.*'
# which gives the regex to test the output of interpreting
# the file against.
foreach(testPath ${ColtTestsPath})
	# Read all the lines of the file
	file(STRINGS ${testPath} testLines)
	# Store the first line
	list(GET testLines 0 firstLine)
	# Pop the 2 first characters which are //
	string(REPLACE "//" "" RegexTest ${firstLine})
	# Strip new lines from end of line
	string(REGEX REPLACE "\n$" "" RegexTest "${RegexTes}")

	# Get test name
	get_filename_component(testName ${testPath} NAME_WE)
	string(TOUPPER ${testName} testName)
	# Create test
	add_executable(${testName}
		${ColtHeaders}
		${testPath}
	)
	target_include_directories(${testName} PUBLIC
		"include"
	)
	add_test(NAME ${testName} COMMAND ${testName})
	set_property(TEST ${testName} PROPERTY PASS_REGULAR_EXPRESSION ${RegexTest})
endforeach()