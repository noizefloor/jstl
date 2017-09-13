cmake_minimum_required(VERSION 3.5)
project(TestHost)

set(CMAKE_CXX_STANDARD 14)

include_directories(inc
        ../lib)
link_directories( lib/linux )

set(SOURCE_FILES
        Source/ConveyorTestCase.cpp
        Source/ConveyorFunctionTestCase.cpp
        Source/PerformanceCounter.cpp
        Source/main.cpp)

set(LIBRARIES
        gtest
        gmock
        pthread)

set(CMAKE_CXX_FLAGS "-Wall -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG  "-Og")

message("${CMAKE_CXX_FLAGS_RELEASE}")

add_executable(TestHost ${SOURCE_FILES})
target_link_libraries(TestHost ${LIBRARIES})