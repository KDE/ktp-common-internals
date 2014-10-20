set (ktploggerplugin_tplogger_SRCS
     tp-logger-plugin.cpp
     pending-tp-logger-dates.cpp
     pending-tp-logger-entities.cpp
     pending-tp-logger-logs.cpp
     pending-tp-logger-search.cpp
     utils.cpp
)

add_library (ktploggerplugin_tplogger MODULE
                 ${ktploggerplugin_tplogger_SRCS}
)

target_link_libraries (ktploggerplugin_tplogger
    ktpcommoninternalsprivate
    ktploggerprivate
    
    ${QT_LIBRARIES}
    
    ${TELEPATHY_QT5_LIBRARIES}
    ${TELEPATHY_LOGGER_QT5_LIBRARIES}
)

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

