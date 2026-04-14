cmake_minimum_required(VERSION 3.10)
project(1c_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_executable(cloudsync main.cpp ${SOURCES})

target_include_directories(cloudsync PRIVATE include)

target_link_libraries(cloudsync PRIVATE CURL::libcurl)

if(UNIX)
    target_link_libraries(cloudsync PRIVATE pthread)
endif()