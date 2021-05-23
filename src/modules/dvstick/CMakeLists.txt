add_library(dvstick SHARED dvstickdriver.cpp dvstickdevice.cpp dvsticksession.cpp protocol.cpp)
set_target_properties(dvstick PROPERTIES VERSION ${PROJECT_VERSION})
install(TARGETS dvstick
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/codecserver
)
