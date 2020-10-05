project(tntcxx)

cmake_minimum_required(VERSION 3.5)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_C_STANDARD 14)
ADD_COMPILE_OPTIONS(-Wall -Wextra -Werror)

add_definitions("-D_GNU_SOURCE")

include_directories(.)
add_executable(BufferUnit.test src/Buffer/Buffer.hpp test/BufferUnitTest.cpp)

enable_testing()
add_test(NAME BufferUnit.test COMMAND BufferUnit.test)
