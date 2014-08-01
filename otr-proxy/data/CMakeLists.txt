
configure_file(org.freedesktop.Telepathy.Client.KTp.Proxy.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.Proxy.service)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.Proxy.service
        DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
install(FILES KTp.Proxy.client DESTINATION ${SHARE_INSTALL_PREFIX}/telepathy/clients/)
