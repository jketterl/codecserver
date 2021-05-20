add_executable(codec-cli codec-cli.cpp cli.cpp)
install(TARGETS codec-cli DESTINATION ${CMAKE_INSTALL_BINDIR})