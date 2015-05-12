set(KTP_LOGGER_PLUGIN_VERSION "1")

set (ktp_logger_private_SRCS
    abstract-logger-plugin.cpp
    log-entity.cpp
    log-manager.cpp
    log-message.cpp
    log-search-hit.cpp
    pending-logger-dates.cpp
    pending-logger-dates-impl.cpp
    pending-logger-entities.cpp
    pending-logger-entities-impl.cpp
    pending-logger-logs.cpp
    pending-logger-logs-impl.cpp
    pending-logger-operation.cpp
    pending-logger-search.cpp
    pending-logger-search-impl.cpp
    scrollback-manager.cpp
    debug.cpp
)

set (ktp_logger_private_HDRS
    abstract-logger-plugin.h
    log-entity.h
    log-manager.h
    log-message.h
    log-search-hit.h
    pending-logger-dates.h
    pending-logger-entities.h
    pending-logger-logs.h
    pending-logger-operation.h
    pending-logger-search.h
    scrollback-manager.h
    ${CMAKE_CURRENT_BINARY_DIR}/ktplogger_export.h
)

add_library (KTpLogger ${ktp_logger_private_SRCS})
add_library (KTp::Logger ALIAS KTpLogger)

generate_export_header(KTpLogger)

set_target_properties(KTpLogger PROPERTIES
                                VERSION ${KTP_VERSION}
                                SOVERSION ${KTP_SONUMBER}
                                EXPORT_NAME Logger
                                DEFINE_SYMBOL MAKE_KTP_LIB)

target_include_directories(KTpLogger PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

target_link_libraries (KTpLogger
                PUBLIC
                       KTp::CommonInternals
                       ${TELEPATHY_LOGGER_LIBRARIES}
                PRIVATE
                       KF5::Service
)

install (TARGETS KTpLogger
         EXPORT KTpTargets
         ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

install (FILES ${ktp_logger_private_HDRS}
         DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KTp/Logger
)

install(FILES ktp_logger_plugin.desktop
        DESTINATION ${SERVICETYPES_INSTALL_DIR}
)


add_subdirectory(plugins)
