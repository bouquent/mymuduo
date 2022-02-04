cmake_minimum_required(VERSION 2.5)
project(mymuduoc++11)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/poller)
aux_source_directory(./src SRC_LIST)
aux_source_directory(./src/poller SRC_LIST)

add_library(mymuduo SHARED ${SRC_LIST})





