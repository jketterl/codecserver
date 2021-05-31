add_executable(codecserver-bin codecserver.cpp server.cpp scanner.cpp clientconnection.cpp serverconfig.cpp socketserver.cpp tcpserver.cpp unixdomainsocketserver.cpp)
set_target_properties(codecserver-bin PROPERTIES OUTPUT_NAME codecserver)
target_compile_definitions(codecserver-bin PRIVATE
    MODULES_PATH="${CMAKE_INSTALL_FULL_LIBDIR}/codecserver"
    CONFIG="${CMAKE_INSTALL_FULL_SYSCONFDIR}/codecserver/codecserver.conf"
)
target_link_libraries(codecserver-bin codecserver dl ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS codecserver-bin DESTINATION ${CMAKE_INSTALL_BINDIR})

configure_file(codecserver.service.in codecserver.service @ONLY)
if (CMAKE_INSTALL_PREFIX STREQUAL "/usr")
    set(SYSTEMD_PREFIX "/lib/systemd/system")
else()
    set(SYSTEMD_PREFIX "${CMAKE_INSTALL_LIBDIR}/systemd/system")
endif()
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/codecserver.service DESTINATION ${SYSTEMD_PREFIX})