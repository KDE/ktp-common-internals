include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR})

set (ktp_common_internals_private_SRCS
     abstract-message-filter.cpp
     account-factory.cpp
     actions.cpp
     capabilities-hack-private.cpp
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
     ${CMAKE_CURRENT_BINARY_DIR}/ktpcommoninternals_export.h
)

if (TelepathyLoggerQt_FOUND)
    set (ktp_private_LIBS
         ${ktp_private_LIBS}
         ${TELEPATHY_LOGGER_LIBRARIES}
         )
endif ()

if (KF5People_FOUND)
    set (ktp_private_LIBS
         ${ktp_private_LIBS}
         KF5::People
         )
endif()

add_library (KTpCommonInternals
                 ${ktp_common_internals_private_SRCS}
)
add_library (KTp::CommonInternals ALIAS KTpCommonInternals)

generate_export_header(KTpCommonInternals BASE_NAME KTpCommonInternals)

#Raise SOVERSION for every 0.x cycle
set_target_properties(KTpCommonInternals PROPERTIES
                                        VERSION ${KTP_VERSION}
                                        SOVERSION ${KTP_SONUMBER}
                                        EXPORT_NAME CommonInternals
                                        DEFINE_SYMBOL MAKE_KTP_LIB)

target_link_libraries (KTpCommonInternals
                PUBLIC
                    KF5::Wallet #included from wallet-interface.h
                    ${TELEPATHY_QT5_LIBRARIES}
                PRIVATE
                    ${ktp_private_LIBS}
                    KF5::KIOWidgets
                    KF5::I18n
                    KF5::IconThemes
                    KF5::Notifications
                    KF5::IconThemes
)
target_include_directories(KTpCommonInternals PUBLIC ${TELEPATHY_QT5_INCLUDE_DIR}) # TODO: Remove when TelepathyQt exports include paths properly

install (TARGETS KTpCommonInternals
         EXPORT KTpTargets
         ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install (FILES ${ktp_common_internals_private_HDRS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/KTp
)

add_subdirectory(Declarative)
add_subdirectory(Models)
add_subdirectory(Widgets)
add_subdirectory(Logger)

if (OTR_LIBS_FOUND)
    add_subdirectory(OTR)
endif (OTR_LIBS_FOUND)

# API docs
find_package(Doxygen)

if(DOXYGEN_EXECUTABLE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  add_custom_target(
    apidox
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile)
endif()
