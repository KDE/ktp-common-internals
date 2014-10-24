include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_otr_private_SRCS
    ${KTP_GLOBAL_SOURCES}
    channel-adapter.cpp
    channel-proxy-interface.cpp
    proxy-service-interface.cpp
    types.cpp
    utils.cpp
)

set (ktp_otr_private_HDRS
    channel-adapter.h
    channel-proxy-interface.h
    constants.h
    proxy-service-interface.h
    types.h
    utils.h
)

kde4_add_library (ktpotrprivate SHARED
                 ${ktp_otr_private_SRCS}
)

set_target_properties(ktpotrprivate PROPERTIES
                                       VERSION ${KTP_VERSION}
                                       SOVERSION ${KTP_SONUMBER}
                                       DEFINE_SYMBOL MAKE_KTP_LIB)

target_link_libraries (ktpotrprivate
                       ${KDE4_KDECORE_LIBS}
                       ${TELEPATHY_QT4_LIBRARIES}
)

install (TARGETS ktpotrprivate
         DESTINATION ${LIB_INSTALL_DIR}
)

install (FILES ${ktp_otr_private_HDRS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/KTp/OTR
)
