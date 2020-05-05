cmake_minimum_required(VERSION 3.16)
project(myAlgo)

# SET VARS
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_LINK_EXECUTABLE ${CMAKE_CXX_LINK_EXECUTABLE})
set(DIR_INCLUDE "/home/berat/include/")
set(LIBS "/home/berat/lib/libtrading.so")

add_library(die SHARED
        die.h
        die.c)
set_property(TARGET die PROPERTY C_STANDARD 99)

add_library(testtrading SHARED
        test.h
        test.c)
set_property(TARGET testtrading PROPERTY C_STANDARD 99)

add_library(fix_common SHARED
        fix_common.h
        fix_common.c)
set_property(TARGET fix_common PROPERTY C_STANDARD 99)

add_library(nanolog SHARED
        NanoLog.hpp
        NanoLog.cpp)

add_executable(myAlgo main.cpp)

# Set header directories ( from VARS )
target_include_directories(myAlgo PUBLIC ${DIR_INCLUDE})
target_include_directories(die PUBLIC ${DIR_INCLUDE})
target_include_directories(testtrading PUBLIC ${DIR_INCLUDE})
target_include_directories(fix_common PUBLIC ${DIR_INCLUDE})

#Link libraries
target_link_libraries(myAlgo PUBLIC ${LIBS} -lm die testtrading fix_common nanolog)
target_link_libraries(die PUBLIC ${LIBS})
target_link_libraries(testtrading PUBLIC ${LIBS})
target_link_libraries(fix_common PUBLIC ${LIBS})