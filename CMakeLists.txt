cmake_minimum_required(VERSION 3.2)
project(dyna2lz)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(DYNA2LZ_SOURCE_FILES
        src/d3plot.h
        src/d3plot.cpp
        src/options.h
        src/options.cpp
    )

set(LSDT_INFO_SOURCE_FILES
        src/lsdt-info.cpp
    )

set(LSDT_DUMP_SOURCE_FILES
        src/lsdt-dump.cpp
    )

add_definitions( 
    -DLSDBINOUT_EXPORTS
    -DBE_QUIET
    -DBUILD_LSD_BINOUT
    -DMAC_OSX
    )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable(lsdt-info ${DYNA2LZ_SOURCE_FILES} ${LSDT_INFO_SOURCE_FILES})
add_executable(lsdt-dump ${DYNA2LZ_SOURCE_FILES} ${LSDT_DUMP_SOURCE_FILES})
