add_library(ambe3k SHARED ambe3kdriver.cpp ambe3kdevice.cpp ambe3ksession.cpp protocol.cpp blockingqueue.cpp channel.cpp queueworker.cpp udevmonitor.cpp ambe3kregistry.cpp)
target_link_libraries(ambe3k codecserver ${CMAKE_THREAD_LIBS_INIT} ${UDEV_LIBRARIES})
target_include_directories(ambe3k PUBLIC ${UDEV_INCLUDE_DIRS})
install(TARGETS ambe3k
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/codecserver
)
