project(telepathy-common-internals)

set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

set (KTELEPATHY_VERSION "0.3.0")
# Bump for every 0.x release, or whenever BC changes
set (KTELEPATHY_SONUMBER 0)

find_package (KDE4 REQUIRED)
find_package (TelepathyQt4 0.7.1 REQUIRED)

# set some default settings
include (KDE4Defaults)

# make some more macros available
include (MacroLibrary)

add_definitions (${KDE4_DEFINITIONS}
                 -DQT_NO_CAST_FROM_ASCII
                 -DQT_NO_KEYWORDS
)

include_directories (${KDE4_INCLUDES}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
)

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
)

kde4_add_library (telepathykdecommoninternalsprivate SHARED
                 ${telepathy_common_internals_private_SRCS}
)

#Raise SOVERSION for every 0.x cycle
set_target_properties(telepathykdecommoninternalsprivate PROPERTIES
                                                         VERSION ${KTELEPATHY_VERSION}
                                                         SOVERSION ${KTELEPATHY_SONUMBER}
                     )

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


add_subdirectory(models)
