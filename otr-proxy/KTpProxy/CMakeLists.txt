set(ktp-proxy_SRCS
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
        otr-config.cpp
        otr-utils.cpp
)

kde4_add_kcfg_files(ktp-proxy_SRCS ktp-proxy-config.kcfgc)

kde4_add_library(ktp-proxy-lib STATIC ${ktp-proxy_SRCS})

set(ktp-proxy_LIBS
        ${KDE4_KDECORE_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KIO_LIBS}
        ${TELEPATHY_QT4_LIBRARIES}
        ${TELEPATHY_QT4_SERVICE_LIBRARIES}
        ${LIBOTR_LIBRARY}
        ${LIBGCRYPT_LIBRARIES}
        ktpcommoninternalsprivate
        ktpotrprivate
)

target_link_libraries(ktp-proxy-lib ${ktp-proxy_LIBS})

kde4_add_executable(ktp-proxy main.cpp)
target_link_libraries(ktp-proxy ktp-proxy-lib)

install(TARGETS ktp-proxy DESTINATION ${LIBEXEC_INSTALL_DIR})
install(FILES ktp-proxy-config.kcfg DESTINATION ${KCFG_INSTALL_DIR})
