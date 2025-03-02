cmake_minimum_required(VERSION 3.14)
project(HFTOrderBook CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the source files
set(SOURCES
    main.cpp
    order_book_manager.cpp
    order_book.cpp
    order_pool.cpp
    price_level.cpp
)

# Add the header files
set(HEADERS
    enums.hpp
    order.hpp
    order_pool.hpp
    order_book.hpp
    order_book_manager.hpp
    price_level.hpp
)

# Create the main executable
add_executable(HFTOrderBook ${SOURCES} ${HEADERS})

# Include directories (if any)
target_include_directories(HFTOrderBook PRIVATE ${CMAKE_SOURCE_DIR})

# Enable warnings and set optimization level
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(HFTOrderBook PRIVATE -Wall -Wextra -O2)
endif()

# Add Google Test as a submodule
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Add a test executable
set(TEST_SOURCES
    test/test_order_book.cpp
    test/simple_tests.cpp
)

# Create a test executable (exclude main.cpp)
add_executable(HFTOrderBookTests ${TEST_SOURCES} order_book_manager.cpp order_book.cpp order_pool.cpp price_level.cpp ${HEADERS})

# Link the test executable with Google Test
target_link_libraries(HFTOrderBookTests PRIVATE gtest_main)

# Include directories for tests
target_include_directories(HFTOrderBookTests PRIVATE ${CMAKE_SOURCE_DIR})

# Enable CTest
enable_testing()

# Add the test executable to CTest
add_test(NAME HFTOrderBookTests COMMAND HFTOrderBookTests)

# Add the test executable to CTest
include(GoogleTest)
gtest_discover_tests(HFTOrderBookTests)
