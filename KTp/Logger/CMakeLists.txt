set(KTP_LOGGER_PLUGIN_VERSION "1")

include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_logger_private_SRCS
    ${KTP_GLOBAL_SOURCES}
    abstract-logger-plugin.cpp
    log-entity.cpp
    log-manager.cpp
    pending-logger-dates.cpp
    pending-logger-dates-impl.cpp
    pending-logger-entities.cpp
    pending-logger-entities-impl.cpp
    pending-logger-logs.cpp
    pending-logger-logs-impl.cpp
    pending-logger-operation.cpp
)

set (ktp_logger_private_HDRS
    abstract-logger-plugin.h
    log-entity.h
    log-manager.h
    pending-logger-dates.h
    pending-logger-entities.h
    pending-logger-operation.h
)

set (ktp_logger_private_LIBS
                       ${KDE4_KDECORE_LIBS}
                       ${TELEPATHY_QT4_LIBRARIES}
)

kde4_add_library (ktploggerprivate SHARED
                 ${ktp_logger_private_SRCS}
)

set_target_properties(ktploggerprivate PROPERTIES
                                       VERSION ${KTP_VERSION}
                                       SOVERSION ${KTP_SONUMBER}
                                       DEFINE_SYMBOL MAKE_KTP_LIB)

target_link_libraries (ktploggerprivate
                       ${ktp_logger_private_LIBS}
)

install (TARGETS ktploggerprivate
         DESTINATION ${LIB_INSTALL_DIR}
)

install (FILES ${ktp_logger_private_HDRS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/KTp/Logger
)


add_subdirectory(plugins)
