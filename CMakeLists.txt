cmake_minimum_required (VERSION 3.0)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project (owrx-connector VERSION 0.1.0)
add_definitions(-DVERSION="${PROJECT_VERSION}-dev")

enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)

include(GNUInstallDirs)

find_package(Threads REQUIRED)

SET(CMAKE_CXX_FLAGS_DEBUG "-g -O3")
SET(CMAKE_C_FLAGS_DEBUG "-g -O3")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_C_FLAGS_RELEASE "-O3")

add_subdirectory(src/server)
