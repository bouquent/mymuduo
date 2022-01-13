cmake_minimum_required(VERSION 2.5)
project(mymuduoc++11)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

aux_source_directory(. SRC_LIST)

add_library(mymuduo SHARED ${SRC_LIST})






#cmake_minimum_required(VERSION 2.5)
#project(mymuduoc++11)

#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FALGS} -g)

#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#aux_source_directory(. SRC_LIST)

#add_library(mymuduo SHARED ${SRC_LIST})




