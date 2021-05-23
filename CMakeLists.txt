cmake_minimum_required (VERSION 3.6)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project (owrx-connector VERSION 0.1.0)
add_definitions(-DVERSION="${PROJECT_VERSION}-dev")

enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-rdynamic")

include(GNUInstallDirs)
include(FindProtobuf)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

SET(CMAKE_CXX_FLAGS_DEBUG "-g -O3")
SET(CMAKE_C_FLAGS_DEBUG "-g -O3")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_C_FLAGS_RELEASE "-O3")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src/proto)
add_subdirectory(src/lib)
add_subdirectory(src/server)
add_subdirectory(src/app)
add_subdirectory(src/modules)