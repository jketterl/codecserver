file(GLOB CODECSERVER_PROTO_FILES "*.proto")
add_library(codecserver_proto OBJECT ${CODECSERVER_PROTO_FILES})
target_link_libraries(codecserver_proto PUBLIC protobuf::libprotobuf)
protobuf_generate(TARGET codecserver_proto LANGUAGE cpp)

install(FILES ${CODECSERVER_PROTO_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/codecserver/proto)

install(FILES ${PROTO_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/codecserver/proto)
