cmake_minimum_required(VERSION 3.6)
project(async_nchan_publisher)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
find_package( Boost COMPONENTS system filesystem  REQUIRED )


set(SOURCE_FILES main.cpp)

include_directories(${Boost_INCLUDE_DIR})
add_executable(async_nchan_publisher ${SOURCE_FILES})
target_link_libraries(async_nchan_publisher ${Boost_LIBRARIES})