cmake_minimum_required(VERSION 3.5)
project(myapp)

find_package(vsomeip3 REQUIRED)
include_directories(/usr/local/include)
add_executable(myapp main.cpp)

target_link_libraries(myapp 
vsomeip3
pthread
)

