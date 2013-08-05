set(ktpwinlogger_SRCS
    main.cpp
    handler.cpp
    db.cpp
)

kde4_add_executable (ktpwinlogger
                     ${ktpwinlogger_SRCS}
)

target_link_libraries(ktpwinlogger
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTDBUS_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${TELEPATHY_QT4_LIBRARIES}
                      ${KTP_LIBRARIES}
)

configure_file(org.freedesktop.Telepathy.Client.KTp.WinLogger.service.in
               org.freedesktop.Telepathy.Client.KTp.WinLogger.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.WinLogger.service
        DESTINATION ${DBUS_SERVICES_INSTALL_DIR})

install(FILES KTp.WinLogger.client DESTINATION ${CMAKE_INSTALL_PREFIX}/share/telepathy/clients/)

install( TARGETS ktpwinlogger DESTINATION ${LIBEXEC_INSTALL_DIR})
