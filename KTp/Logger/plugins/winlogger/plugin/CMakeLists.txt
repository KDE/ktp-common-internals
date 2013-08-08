set (ktploggerplugin_winlogger_SRCS
     win-logger-plugin.cpp
     pending-win-logger-dates.cpp
     pending-win-logger-entities.cpp
     pending-win-logger-logs.cpp
     ../logger/db.cpp
)

kde4_add_plugin (ktploggerplugin_winlogger
                 ${ktploggerplugin_winlogger_SRCS}
)

target_link_libraries (ktploggerplugin_winlogger
    ktpcommoninternalsprivate
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTSQL_LIBRARIES}
    ${TELEPATHY_QT4_LIBRARIES}
    ${TELEPATHY_LOGGER_QT4_LIBRARIES}
    ktpcommoninternalsprivate
    ktploggerprivate
)

# Install:
install (TARGETS ktploggerplugin_winlogger
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktploggerplugin_winlogger.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktploggerplugin_winlogger.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktploggerplugin_winlogger.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

