cmake_minimum_required(VERSION 3.3)
project(Project4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES p4tester.cpp DiskMultiMap.cpp IntelWeb.cpp)
add_executable(Project4 ${SOURCE_FILES})