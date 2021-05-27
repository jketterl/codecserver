add_library(codecserver SHARED connection.cpp)
target_link_libraries(codecserver codecserver_proto ${Protobuf_LIBRARIES})
set_target_properties(codecserver PROPERTIES VERSION ${PROJECT_VERSION})
file(GLOB CODECSERVER_HEADERS "${PROJECT_SOURCE_DIR}/include/*.hpp" "${PROJECT_SOURCE_DIR}/proto/*.proto" "${codecserver_proto_includes}/*.pb.h")
set_target_properties(codecserver PROPERTIES PUBLIC_HEADER "${CODECSERVER_HEADERS}")
target_include_directories(codecserver PUBLIC ${codecserver_proto_includes})
install(TARGETS codecserver
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/codecserver
)

configure_file(codecserver.pc.in codecserver.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/codecserver.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)