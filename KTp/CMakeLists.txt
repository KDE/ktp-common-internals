include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR})

set (ktp_common_internals_private_SRCS
     ${KTP_GLOBAL_SOURCES}
     abstract-message-filter.cpp
     account-factory.cpp
     actions.cpp
     capabilities-hack-private.cpp
     contact-info-dialog.cpp
     circular-countdown.cpp
     contact.cpp
     contact-factory.cpp
     core.cpp
     debug.cpp
     error-dictionary.cpp
     global-contact-manager.cpp
     global-presence.cpp
     logs-importer.cpp
     logs-importer-private.cpp
     message.cpp
     message-context.cpp
     message-escape-filter.cpp
     message-filter-config-manager.cpp
     message-processor.cpp
     outgoing-message.cpp
     persistent-contact.cpp
     presence.cpp
     service-availability-checker.cpp
     telepathy-handler-application.cpp
     text-parser.cpp
     pending-wallet.cpp
     wallet-interface.cpp
     wallet-utils.cpp
)

set (ktp_common_internals_private_HDRS
     abstract-message-filter.h
     actions.h
     circular-countdown.h
     contact.h
     contact-info-dialog.h
     contact-factory.h
     core.h
     debug.h
     error-dictionary.h
     global-contact-manager.h
     global-presence.h
     logs-importer.h
     message-filter-config-manager.h
     message.h
     message-context.h
     message-processor.h
     outgoing-message.h
     persistent-contact.h
     presence.h
     service-availability-checker.h
     telepathy-handler-application.h
     text-parser.h
     pending-wallet.h
     wallet-interface.h
     wallet-utils.h
     types.h
     ktp-export.h
)

set (ktp_private_LIBS
                       ${KDE4_KDECORE_LIBS}
                       ${KDE4_KIO_LIBS}
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${KDE4_KDEUI_LIBS}
                       ${KDEPIMLIBS_KABC_LIBS}
)

if (TELEPATHY_LOGGER_QT4_FOUND)
    set (ktp_private_LIBS
         ${ktp_private_LIBS}
         ${TELEPATHY_LOGGER_QT4_LIBRARIES}
         )
endif (TELEPATHY_LOGGER_QT4_FOUND)

if (KPEOPLE_FOUND)
    include_directories(${KPEOPLE_INCLUDES})

    set (ktp_private_LIBS
         ${ktp_private_LIBS}
         ${KPEOPLE_LIBS}
         )
endif(KPEOPLE_FOUND)

kde4_add_library (ktpcommoninternalsprivate SHARED
                 ${ktp_common_internals_private_SRCS}
)

#Raise SOVERSION for every 0.x cycle
set_target_properties(ktpcommoninternalsprivate PROPERTIES
                                                         VERSION ${KTP_VERSION}
                                                         SOVERSION ${KTP_SONUMBER}
                                                         DEFINE_SYMBOL MAKE_KTP_LIB)

target_link_libraries (ktpcommoninternalsprivate
                      ${ktp_private_LIBS}
)

install (TARGETS ktpcommoninternalsprivate
         DESTINATION ${LIB_INSTALL_DIR}
)

install (FILES ${ktp_common_internals_private_HDRS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/KTp
)

add_subdirectory(Declarative)
add_subdirectory(Models)
add_subdirectory(Widgets)
add_subdirectory(Logger)
add_subdirectory(OTR)

# API docs
find_package(Doxygen)

if(DOXYGEN_EXECUTABLE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  add_custom_target(
    apidox
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile)
endif(DOXYGEN_EXECUTABLE)
