cmake_minimum_required(VERSION 3.5)

# Compiler cache

if (${CMAKE_GENERATOR} STREQUAL "Unix Makefiles" OR ${CMAKE_GENERATOR} STREQUAL "Ninja")

    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_PROGRAM})

        message("Using ccache: ${CCACHE_PROGRAM}")

    else(CCACHE_PROGRAM)

        message("No supported compiler cache found.")

    endif()

endif()


project(jstl)

# Jan Schwers Template Library

add_library(jstl INTERFACE)

target_compile_features(jstl INTERFACE cxx_std_14)

target_include_directories(jstl
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/jstl>)

install(DIRECTORY include/ DESTINATION include/jstl)

install(TARGETS jstl EXPORT jstl-config DESTINATION jstl/include)

install(EXPORT jstl-config DESTINATION share/jstl/cmake)

# Documentation

option(BUILD_DOC "Build documentation" ON)
find_package(Doxygen
        REQUIRED dot
        OPTIONAL_COMPONENTS mscgen dia)

if(DOXYGEN_FOUND)

    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc)
    set(HTML_EXTRA_STYLESHEET ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/doxygen_extra.css)

    doxygen_add_docs(doxygen
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            COMMENT "Jan Schwers Template Library API documentation")

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html DESTINATION share/doc)

else(DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
endif()


# Unit Tests

if(MSVC)
    set(gtest_force_shared_crt ON)
endif(MSVC)

add_subdirectory(googletest)

enable_testing()

add_executable(jstlTestHost
        TestHost/ConveyorTestCase.cpp
        TestHost/ConveyorFunctionTestCase.cpp
        TestHost/FunctionTraitsTestCase.cpp
        TestHost/main.cpp TestHost/FlatSetTestCase.cpp)

target_compile_features(jstlTestHost PRIVATE cxx_std_14)

target_include_directories(jstlTestHost PRIVATE TestHost)

target_link_libraries(jstlTestHost
    PRIVATE
        jstl
        gtest
        gmock)

add_test(NAME jstl-UnitTests COMMAND jstlTestHost --gtest_filter=UnitTest*)

# Benchmarks

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Don't build benchmark gtests." FORCE)
add_subdirectory(google-benchmark)

find_package(Threads)
find_package(Boost 1.56.0 REQUIRED)

add_executable(jstlBenchmark
        Benchmark/TestValue.h
        Benchmark/main.cpp
        Benchmark/ConveyorFunctionBenchmark.cpp
        Benchmark/ConveyorBenchmark.cpp
        Benchmark/SetBenchmark.cpp
        Benchmark/MapBenchmark.cpp)

target_compile_features(jstlBenchmark PRIVATE cxx_std_14)

target_include_directories(jstlTestHost PRIVATE Benchmark)

target_link_libraries(jstlBenchmark
    PRIVATE
        benchmark
        jstl
        Boost::boost
        ${CMAKE_THREAD_LIBS_INIT})