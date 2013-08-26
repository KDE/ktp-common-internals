set (ktploggerplugin_tplogger_SRCS
     tp-logger-plugin.cpp
     pending-tp-logger-dates.cpp
     pending-tp-logger-entities.cpp
     pending-tp-logger-logs.cpp
     pending-tp-logger-search.cpp
     utils.cpp
)

kde4_add_plugin (ktploggerplugin_tplogger
                 ${ktploggerplugin_tplogger_SRCS}
)

target_link_libraries (ktploggerplugin_tplogger
    ktpcommoninternalsprivate
    ktploggerprivate
    ${QT_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${TELEPATHY_LOGGER_QT4_LIBRARIES}
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

