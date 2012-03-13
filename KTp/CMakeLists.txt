include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR})

set (ktp_common_internals_private_SRCS
     circular-countdown.cpp
     debug.cpp
     error-dictionary.cpp
     global-presence.cpp
     presence.cpp
     service-availability-checker.cpp
     telepathy-handler-application.cpp
     text-parser.cpp
     wallet-interface.cpp
)

set (ktp_common_internals_private_HDRS
     circular-countdown.h
     debug.h
     error-dictionary.h
     global-presence.h
     presence.h
     service-availability-checker.h
     telepathy-handler-application.h
     text-parser.h
     wallet-interface.h

     ktp-export.h
)

kde4_add_library (ktpcommoninternalsprivate SHARED
                 ${ktp_common_internals_private_SRCS}
)

#Raise SOVERSION for every 0.x cycle
set_target_properties(ktpcommoninternalsprivate PROPERTIES
                                                         VERSION ${KTP_VERSION}
                                                         SOVERSION ${KTP_SONUMBER}
                                                         DEFINE_SYMBOL MAKE_KTP_LIB)

target_link_libraries (ktpcommoninternalsprivate
                       ${KDE4_KDECORE_LIBS}
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${KDE4_KDEUI_LIBS}
)

install (TARGETS ktpcommoninternalsprivate
         DESTINATION ${LIB_INSTALL_DIR}
)

install (FILES ${ktp_common_internals_private_HDRS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/KTp
)

add_subdirectory(Models)
add_subdirectory(Widgets)
