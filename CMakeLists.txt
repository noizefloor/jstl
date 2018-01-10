include(cmake/PrecompiledHeader.cmake)

cmake_minimum_required(VERSION 3.5)

project(jstl)

add_library(jstl INTERFACE)

target_compile_features(jstl INTERFACE cxx_std_14)

target_include_directories(jstl
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:jstl/include>)

install(TARGETS jstl EXPORT jstl-config DESTINATION jstl/include)

install(EXPORT jstl-config DESTINATION share/jstl/cmake)


add_subdirectory(googletest)

enable_testing()

add_executable(jstlTestHost
        TestHost/stdafx.h
        TestHost/stdafx.cpp
        TestHost/ConveyorTestCase.cpp
        TestHost/ConveyorFunctionTestCase.cpp
        TestHost/FunctionTraitsTestCase.cpp
        TestHost/PerformanceCounter.cpp
        TestHost/main.cpp)

target_compile_features(jstlTestHost PRIVATE cxx_std_14)

add_precompiled_header(jstlTestHost stdafx.h)

target_include_directories(jstlTestHost PRIVATE TestHost/inc)

target_link_libraries(jstlTestHost
    PRIVATE
        jstl
        gtest
        gmock)

add_test(NAME UnitTests COMMAND jstlTestHost --gtest_filter=UnitTest*)