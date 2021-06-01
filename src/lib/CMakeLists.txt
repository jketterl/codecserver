set(CMAKE_POSITION_INDEPENDENT_CODE On)

add_subdirectory(proto)

add_library(codecserver SHARED connection.cpp config.cpp registry.cpp device.cpp)
target_link_libraries(codecserver ${Protobuf_LIBRARIES} codecserver_proto)
set_target_properties(codecserver PROPERTIES VERSION ${PROJECT_VERSION})
file(GLOB CODECSERVER_HEADERS "${PROJECT_SOURCE_DIR}/include/*.hpp")
set_target_properties(codecserver PROPERTIES PUBLIC_HEADER "${CODECSERVER_HEADERS}")

# for the protobuf includes
target_include_directories(codecserver PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS codecserver
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/codecserver
)

configure_file(codecserver.pc.in codecserver.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/codecserver.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
