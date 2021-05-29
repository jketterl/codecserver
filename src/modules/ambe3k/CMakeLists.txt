add_library(ambe3k SHARED ambe3kdriver.cpp ambe3kdevice.cpp ambe3ksession.cpp protocol.cpp blockingqueue.cpp)
target_link_libraries(ambe3k codecserver ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(ambe3k PROPERTIES VERSION ${PROJECT_VERSION})
install(TARGETS ambe3k
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/codecserver
)
