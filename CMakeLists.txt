cmake_minimum_required(VERSION 3.16)
project(myAlgo)

set(CMAKE_CXX_STANDARD 14)
add_library(die SHARED
        die.h
        die.c)

add_library(testtrading SHARED
        test.h
        test.c)

add_library(fix_common SHARED
        fix_common.h
        fix_common.c)

add_library(fastlog SHARED
        fastlog_filebuffer.h
        fastlog_filebuffer.cpp
        fastlog_utils.h)

# SET VARS
set(DIR_INCLUDE "/home/berat/include/")
set(LIBS "/home/berat/lib/libtrading.so")

add_executable(myAlgo main.cpp)

# Set header directories ( from VARS )
target_include_directories(myAlgo PUBLIC ${DIR_INCLUDE})
target_include_directories(die PUBLIC ${DIR_INCLUDE})
target_include_directories(testtrading PUBLIC ${DIR_INCLUDE})
target_include_directories(fix_common PUBLIC ${DIR_INCLUDE})

#Link libraries
target_link_libraries(myAlgo PUBLIC ${LIBS} -lm die testtrading fix_common fastlog)
target_link_libraries(die PUBLIC ${LIBS})
target_link_libraries(testtrading PUBLIC ${LIBS})
target_link_libraries(fix_common PUBLIC ${LIBS})