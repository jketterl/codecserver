add_library(codecserver SHARED connection.cpp)
target_link_libraries(codecserver codecserver_proto ${Protobuf_LIBRARIES})
target_include_directories(codecserver PUBLIC ${codecserver_proto_includes})
install(TARGETS codecserver
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    # PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/owrx
)
