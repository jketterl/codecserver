cmake_minimum_required (VERSION 3.6)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project (codecserver VERSION 0.2.0)
add_definitions(-DVERSION="${PROJECT_VERSION}-dev")

enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

include(GNUInstallDirs)
include(FindProtobuf)

find_package(Threads REQUIRED)
find_package(Protobuf 3.0 REQUIRED)
find_package(udev REQUIRED)

SET(CMAKE_CXX_FLAGS_DEBUG "-g -O3 -rdynamic")
SET(CMAKE_C_FLAGS_DEBUG "-g -O3")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -rdynamic")
SET(CMAKE_C_FLAGS_RELEASE "-O3")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src/lib)
add_subdirectory(src/server)
add_subdirectory(src/modules)