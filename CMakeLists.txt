cmake_minimum_required(VERSION 3.14)
project(LocalSearchEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(search_core
    src/inverted_index.cpp
    src/search_server.cpp
)

add_executable(search_engine
    src/main.cpp
)

target_link_libraries(search_engine PRIVATE search_core)

if(MSVC)
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    
    target_compile_options(search_core PRIVATE /W4 /WX)
    target_compile_options(search_engine PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(search_core PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(search_engine PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(search_tests
    test/test_index.cpp
)

target_link_libraries(search_tests PRIVATE search_core gtest_main)