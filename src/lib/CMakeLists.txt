set(CMAKE_POSITION_INDEPENDENT_CODE On)

add_subdirectory(proto)

add_library(codecserver SHARED connection.cpp config.cpp registry.cpp device.cpp $<TARGET_OBJECTS:codecserver_proto>)
target_link_libraries(codecserver PUBLIC protobuf::libprotobuf)
set_target_properties(codecserver PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
file(GLOB CODECSERVER_HEADERS "${PROJECT_SOURCE_DIR}/include/*.hpp")
set_target_properties(codecserver PROPERTIES PUBLIC_HEADER "${CODECSERVER_HEADERS}")

# for the protobuf includes
target_include_directories(codecserver PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> $<INSTALL_INTERFACE:>)

install(TARGETS codecserver
    EXPORT CodecServerTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/codecserver
)

configure_file(codecserver.pc.in codecserver.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/codecserver.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(EXPORT CodecServerTargets
    FILE CodecServerTargets.cmake
    NAMESPACE CodecServer::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CodecServer
)

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/CodecServerConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CodecServer
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CodecServerConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CodecServerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CodecServerConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CodecServer
)
