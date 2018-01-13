include(cmake/PrecompiledHeader.cmake)

cmake_minimum_required(VERSION 3.5)

project(jstl)

# Jan Schwers Template Library

find_package(boost 1.5.6.0
        REQUIRED
        COMPONENTS container)

add_library(jstl INTERFACE)

target_compile_features(jstl INTERFACE cxx_std_14)

target_include_directories(jstl
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/jstl>)

target_link_libraries(jstl INTERFACE ${Boost_LIBRARIES})

install(DIRECTORY include/ DESTINATION include/jstl)

install(TARGETS jstl EXPORT jstl-config DESTINATION jstl/include)

install(EXPORT jstl-config DESTINATION share/jstl/cmake)

# Documentation

option(BUILD_DOC "Build documentation" ON)
find_package(Doxygen
        REQUIRED dot
        OPTIONAL_COMPONENTS mscgen dia)

if (DOXYGEN_FOUND)

    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc)
    set(HTML_EXTRA_STYLESHEET ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/doxygen_extra.css)

    doxygen_add_docs(doxygen
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            COMMENT "Jan Schwers Template Library API documentation")

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html DESTINATION share/doc)
else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
endif()


# Unit Tests

add_subdirectory(googletest)

enable_testing()

add_executable(jstlTestHost
        TestHost/stdafx.h
        TestHost/stdafx.cpp
        TestHost/ConveyorTestCase.cpp
        TestHost/ConveyorFunctionTestCase.cpp
        TestHost/FunctionTraitsTestCase.cpp
        TestHost/PerformanceCounter.cpp
        TestHost/main.cpp TestHost/FlatSetTestCase.cpp)

target_compile_features(jstlTestHost PRIVATE cxx_std_14)

add_precompiled_header(jstlTestHost
        stdafx.h
        SOURCE_CXX stdafx.cpp
        FORCEINCLUDE)

target_include_directories(jstlTestHost PRIVATE TestHost)

target_link_libraries(jstlTestHost
    PRIVATE
        jstl
        gtest
        gmock)

add_test(NAME UnitTests COMMAND jstlTestHost --gtest_filter=UnitTest*)

# Benchmarks

find_package(benchmark)

add_executable(jstlBenchmark
        Benchmark/stdafx.h
        Benchmark/stdafx.cpp
        Benchmark/main.cpp
        Benchmark/ConveyorFunctionBenchmark.cpp
        Benchmark/ConveyorBenchmark.cpp
        Benchmark/FlatSetBenchmark.cpp
        Benchmark/FlatMapBenchmark.cpp)

target_compile_features(jstlBenchmark PRIVATE cxx_std_14)

add_precompiled_header(jstlBenchmark
        stdafx.h
        SOURCE_CXX stdafx.cpp
        FORCEINCLUDE)

target_include_directories(jstlTestHost PRIVATE Benchmark)

target_link_libraries(jstlBenchmark
    PRIVATE
        benchmark
        jstl)