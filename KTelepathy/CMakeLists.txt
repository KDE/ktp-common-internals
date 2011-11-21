include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR})

set (telepathy_common_internals_private_SRCS
     circular-countdown.cpp
     error-dictionary.cpp
     global-presence.cpp
     presence.cpp
     service-availability-checker.cpp
     telepathy-handler-application.cpp
     text-parser.cpp
     wallet-interface.cpp
)

set (telepathy_common_internals_private_HDRS
     circular-countdown.h
     error-dictionary.h
     global-presence.h
     presence.h
     service-availability-checker.h
     telepathy-handler-application.h
     text-parser.h
     wallet-interface.h

     ktelepathy-export.h
)

kde4_add_library (telepathykdecommoninternalsprivate SHARED
                 ${telepathy_common_internals_private_SRCS}
)

#Raise SOVERSION for every 0.x cycle
set_target_properties(telepathykdecommoninternalsprivate PROPERTIES
                                                         VERSION ${KTELEPATHY_VERSION}
                                                         SOVERSION ${KTELEPATHY_SONUMBER}
                                                         DEFINE_SYMBOL MAKE_KTELEPATHY_LIB)

target_link_libraries (telepathykdecommoninternalsprivate
                       ${KDE4_KDECORE_LIBS}
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${KDE4_KDEUI_LIBS}
)

install (TARGETS telepathykdecommoninternalsprivate
         DESTINATION ${LIB_INSTALL_DIR}
)

install (FILES ${telepathy_common_internals_private_HDRS}
         DESTINATION ${TELEPATHY_QT4_INCLUDE_DIR}/KTelepathy
)


add_subdirectory(Models)
