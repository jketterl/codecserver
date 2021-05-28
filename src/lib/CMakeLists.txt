set(CMAKE_POSITION_INDEPENDENT_CODE On)
file(GLOB CODECSERVER_PROTO_FILES "*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CODECSERVER_PROTO_FILES})
set(codecserver_proto_includes ${CMAKE_CURRENT_BINARY_DIR})

add_library(codecserver SHARED connection.cpp config.cpp ${CODECSERVER_PROTO_FILES})
protobuf_generate(TARGET codecserver)
target_link_libraries(codecserver ${Protobuf_LIBRARIES})
set_target_properties(codecserver PROPERTIES VERSION ${PROJECT_VERSION})
file(GLOB CODECSERVER_HEADERS "${PROJECT_SOURCE_DIR}/include/*.hpp")
set_target_properties(codecserver PROPERTIES PUBLIC_HEADER "${CODECSERVER_HEADERS}")
target_include_directories(codecserver PUBLIC ${codecserver_proto_includes})
install(TARGETS codecserver
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/codecserver
)

file(GLOB CODECSERVER_PROTO_FILES "*.proto")
install(FILES ${CODECSERVER_PROTO_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/codecserver/proto)

install(FILES ${PROTO_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/codecserver/proto)

configure_file(codecserver.pc.in codecserver.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/codecserver.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
