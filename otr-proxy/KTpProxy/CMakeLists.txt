set(ktp-proxy_SRCS
        main.cpp
        proxy-observer.cpp
        proxy-service-adaptee.cpp
        proxy-service.cpp
        svc-channel-proxy.cpp
        svc-proxy-service.cpp
        otr-proxy-channel.cpp
        otr-proxy-channel-adaptee.cpp
        pending-curry-operation.cpp
        otr-session.cpp
        otr-manager.cpp
        otr-message.cpp
)

set(ktp-proxy_LIBS
        ${KDE4_KDECORE_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KIO_LIBS}
        ${TELEPATHY_QT4_LIBRARIES}
        ${TELEPATHY_QT4_SERVICE_LIBRARIES}
        ${LIBOTR_LIBRARY}
        ${LIBGCRYPT_LIBRARIES}
)

kde4_add_executable(ktp-proxy ${ktp-proxy_SRCS})

target_link_libraries(ktp-proxy ${ktp-proxy_LIBS})
install(TARGETS ktp-proxy DESTINATION ${LIBEXEC_INSTALL_DIR})
