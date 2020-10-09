project(tntcxx)

cmake_minimum_required(VERSION 3.5)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_C_STANDARD 17)
ADD_COMPILE_OPTIONS(-Wall -Wextra -Werror)

add_definitions("-D_GNU_SOURCE")

include_directories(./src)
add_executable(BufferUnit.test src/Buffer/Buffer.hpp test/BufferUnitTest.cpp)
add_executable(EncDecUnit.test src/mpp/mpp.hpp test/EncDecTest.cpp)

enable_testing()
add_test(NAME BufferUnit.test COMMAND BufferUnit.test)
add_test(NAME EncDecUnit.test COMMAND EncDecUnit.test)
