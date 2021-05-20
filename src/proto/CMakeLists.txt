add_library(codecserver_proto STATIC handshake.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS handshake.proto)
protobuf_generate(TARGET codecserver_proto)
set(codecserver_proto_includes ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
