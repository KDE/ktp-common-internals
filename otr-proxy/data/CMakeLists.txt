
configure_file(org.freedesktop.Telepathy.Client.KTp.Proxy.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.Proxy.service)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.Proxy.service
        DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
install(FILES KTp.Proxy.client DESTINATION ${KDE_INSTALL_DATAROOTDIR}/telepathy/clients/)
