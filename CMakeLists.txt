cmake_minimum_required(VERSION 3.3)
project(Project4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES DiskMultiMap.cpp IntelWeb.cpp main.cpp)
add_executable(Project4 ${SOURCE_FILES})