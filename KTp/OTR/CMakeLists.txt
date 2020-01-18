include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (ktp_otr_private_HDRS
    channel-adapter.h
    channel-proxy-interface.h
    constants.h
    proxy-service-interface.h
    types.h
    utils.h
    ${CMAKE_CURRENT_BINARY_DIR}/ktpotr_export.h
)

add_library (KTpOTR SHARED
    channel-adapter.cpp
    channel-proxy-interface.cpp
    proxy-service-interface.cpp
    types.cpp
    utils.cpp
    debug.cpp)
add_library(KTp::OTR ALIAS KTpOTR)

set_target_properties(KTpOTR PROPERTIES
                                       VERSION ${KTP_VERSION}
                                       SOVERSION ${KTP_SONUMBER}
                                       EXPORT_NAME OTR
                                       DEFINE_SYMBOL MAKE_KTP_LIB)
generate_export_header(KTpOTR)

target_link_libraries (KTpOTR
                        PUBLIC
                            Qt5::DBus
                            TelepathyQt5::Core
                        PRIVATE
                            KF5::I18n
)

install (TARGETS KTpOTR
         EXPORT KTpTargets
         ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

install (FILES ${ktp_otr_private_HDRS}
         DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KTp/OTR
)
