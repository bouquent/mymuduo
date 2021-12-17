#生成动态库
cmake_minimum_required(VERSION 2.5)
project(muduo)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

aux_source_directory(. SRC_LIST)

add_library(mymuduo SHARED ${SRC_LIST}) 





#生成可执行文件
#cmake_minimum_required(VERSION 2.5)
#project(muduos)

#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#aux_source_directory(. SRC_LIST)

#add_executable(mymuduo ${SRC_LIST})

#target_link_libraries(mymuduo pthread)
