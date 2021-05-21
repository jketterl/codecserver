add_library(dvstick SHARED dvstickcodec.cpp dvsticksession.cpp dvstick.cpp protocol.cpp)
set_target_properties(dvstick PROPERTIES VERSION ${PROJECT_VERSION})
install(TARGETS dvstick
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/codecserver
)
