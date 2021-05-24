add_library(dvstick SHARED dvstickdriver.cpp dvstickdevice.cpp dvsticksession.cpp protocol.cpp blockingqueue.cpp)
target_link_libraries(dvstick codecserver ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(dvstick PROPERTIES VERSION ${PROJECT_VERSION})
install(TARGETS dvstick
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/codecserver
)
