add_executable(codecserver-bin codecserver.cpp server.cpp scanner.cpp registry.cpp clientconnection.cpp serverconfig.cpp)
set_target_properties(codecserver-bin PROPERTIES OUTPUT_NAME codecserver)
target_compile_definitions(codecserver-bin PRIVATE
    MODULES_PATH="${CMAKE_INSTALL_FULL_LIBDIR}/codecserver"
    CONFIG="${CMAKE_INSTALL_FULL_SYSCONFDIR}/codecserver/codecserver.conf"
)
target_link_libraries(codecserver-bin codecserver dl ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS codecserver-bin DESTINATION ${CMAKE_INSTALL_BINDIR})