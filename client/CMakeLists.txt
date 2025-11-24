cmake_minimum_required(VERSION 3.5)
project(myclient)

find_package(vsomeip3 REQUIRED)
include_directories(/usr/local/include)

add_executable(myclient main.cpp)

target_link_libraries(myclient vsomeip3)
