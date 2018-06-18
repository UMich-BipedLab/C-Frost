cmake_minimum_required(VERSION 2.8)
project(frost_ipopt)
set(C_FROST_ROOT_PATH "INSERT_PATH_TO_ROOT_C_FROST")

if(C_FROST_ROOT_PATH STREQUAL "INSERT_PATH_TO_ROOT_C_FROST")
    message(FATAL_ERROR "Please update C_FROST_ROOT_PATH to be the correct path." )
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Wall -Wno-unused-variable -Wno-unused-value -O0 -pipe -Wparentheses -Wreturn-type -Wcast-qual -Wall -Wpointer-arith -Wwrite-strings -Wconversion -Wno-unknown-pragmas -Wno-long-long -fopenmp  -DIPOPT_BUILD -std=c++11 -lpthread")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -Wall -Wno-unused-variable -Wno-unused-value -O0 -pipe -Wparentheses -Wreturn-type -Wcast-qual -Wall -Wpointer-arith -Wwrite-strings -Wconversion -Wno-unknown-pragmas -Wno-long-long -fopenmp  -DIPOPT_BUILD -std=c++11 -lpthread")
set(CMAKE_INSTALL_PREFIX "../")

set(IPOPT_LIBRARY_DIRS /usr/local/lib)
set(IPOPT_INCLUDE_DIRS /usr/local/include/coin)

message("ipopt binary path is: " ${IPOPT_LIBRARY_DIRS} )
message("ipopt include path is: " ${IPOPT_INCLUDE_DIRS} )

include_directories(${IPOPT_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(SYSTEM ${C_FROST_ROOT_PATH}/Linux/include)
#include_directories(${IPOPT_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include "/usr/include/rapidjson"  ${C_FROST_ROOT_PATH}/Linux/include)
link_directories(${IPOPT_LIBRARY_DIRS})


#However, the file(GLOB...) allows for wildcard additions:
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR}  "src/*.cc")
set(MAIN_SOURCES ${C_FROST_ROOT_PATH}/Linux/src/main.cpp ${C_FROST_ROOT_PATH}/Linux/src/IpoptProblem.cpp ${C_FROST_ROOT_PATH}/Linux/src/JacGEvalMultiThread.cpp ${C_FROST_ROOT_PATH}/Linux/src/JacGEvalSingleThread.cpp)


add_executable(program ${MAIN_SOURCES} ${SOURCES})
target_link_libraries(program ipopt)

install (TARGETS program DESTINATION .)
