cmake_minimum_required(VERSION 3.00)
project(redis_test)

set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/local/include/hiredis)
link_directories(/usr/local/lib/hiredis)

add_executable(redis_test main.cpp)

target_link_libraries(redis_test hiredis)