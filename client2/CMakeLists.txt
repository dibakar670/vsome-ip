cmake_minimum_required(VERSION 3.5)
project(myclient2)

find_package(vsomeip3 REQUIRED)
include_directories(/usr/local/include)

add_executable(myclient2 main.cpp)

target_link_libraries(myclient2 vsomeip3)

