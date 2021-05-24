add_executable(codec-cli codec-cli.cpp cli.cpp)
target_link_libraries(codec-cli codecserver)
install(TARGETS codec-cli DESTINATION ${CMAKE_INSTALL_BINDIR})