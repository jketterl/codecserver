add_executable(codecserver codecserver.cpp server.cpp)
install(TARGETS codecserver DESTINATION ${CMAKE_INSTALL_BINDIR})