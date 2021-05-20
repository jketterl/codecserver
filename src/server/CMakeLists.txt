add_executable(codecserver codecserver.cpp server.cpp scanner.cpp registry.cpp connection.cpp)
target_compile_definitions(codecserver PUBLIC MODULES_PATH="${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/codecserver")
target_link_libraries(codecserver dl ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS codecserver DESTINATION ${CMAKE_INSTALL_BINDIR})