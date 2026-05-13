cmake_minimum_required(VERSION 3.10)
project(CloudSync VERSION 1.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)


file(GLOB_RECURSE LIB_SOURCES "src/*.cpp")
add_library(cloudsync_lib STATIC ${LIB_SOURCES})

target_include_directories(cloudsync_lib PUBLIC include)
target_link_libraries(cloudsync_lib PUBLIC CURL::libcurl)

if(UNIX)
    target_link_libraries(cloudsync_lib PUBLIC pthread)
endif()

add_executable(cloudsync main.cpp)
target_link_libraries(cloudsync PRIVATE cloudsync_lib)

# БЛОК GOOGLE TEST 
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()
add_subdirectory(tests)