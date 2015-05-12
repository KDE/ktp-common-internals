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
        ktp-proxy-debug.cpp
)

kconfig_add_kcfg_files(ktp-proxy_SRCS ktp-proxy-config.kcfgc)

add_library(ktp-proxy-lib STATIC ${ktp-proxy_SRCS})

set(ktp-proxy_LIBS
        ${LIBOTR_LIBRARY}
        ${LIBGCRYPT_LIBRARIES}
        ${TELEPATHY_QT5_SERVICE_LIBRARIES}
        KTp::CommonInternals
        KTp::OTR
        KF5::ConfigGui
)

target_link_libraries(ktp-proxy-lib ${ktp-proxy_LIBS})

add_executable(ktp-proxy main.cpp)
target_link_libraries(ktp-proxy ktp-proxy-lib)

install(TARGETS ktp-proxy DESTINATION ${LIBEXEC_INSTALL_DIR})
install(FILES ktp-proxy-config.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
