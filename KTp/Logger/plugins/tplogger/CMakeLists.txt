add_library (ktploggerplugin_tplogger MODULE
     tp-logger-plugin.cpp
     pending-tp-logger-dates.cpp
     pending-tp-logger-entities.cpp
     pending-tp-logger-logs.cpp
     pending-tp-logger-search.cpp
     utils.cpp
)

target_link_libraries (ktploggerplugin_tplogger
    KTp::CommonInternals
    KTp::Logger
    KF5::Service
)
kde_target_enable_exceptions(ktploggerplugin_tplogger PRIVATE)

# Install:
install (TARGETS ktploggerplugin_tplogger
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktploggerplugin_tplogger.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktploggerplugin_tplogger.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktploggerplugin_tplogger.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

