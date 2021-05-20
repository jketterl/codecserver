add_executable(codec-cli codec-cli.cpp cli.cpp)
target_link_libraries(codec-cli codecserver_proto ${Protobuf_LIBRARIES})
target_include_directories(codec-cli PRIVATE ${codecserver_proto_includes})
install(TARGETS codec-cli DESTINATION ${CMAKE_INSTALL_BINDIR})