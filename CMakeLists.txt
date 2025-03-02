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

# Create the executable
add_executable(HFTOrderBook ${SOURCES} ${HEADERS})

# Include directories (if any)
target_include_directories(HFTOrderBook PRIVATE ${CMAKE_SOURCE_DIR})

# Add any necessary libraries (if any)
# target_link_libraries(HFTOrderBook PRIVATE some_library)

# Enable warnings and set optimization level
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(HFTOrderBook PRIVATE -Wall -Wextra -O2)
endif()
