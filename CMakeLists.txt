project(ktp-common-internals)

cmake_minimum_required (VERSION 2.6.3 FATAL_ERROR)

set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)
cmake_policy(SET CMP0002 OLD)

set (KTP_VERSION "0.6.2")
# Bump for every 0.x release, or whenever BC changes
set (KTP_SONUMBER 5)
set (KTP_MESSAGE_FILTER_FRAMEWORK_VERSION "4")

find_package (KDE4 REQUIRED)
find_package (TelepathyQt4 0.9.2.1 REQUIRED)
find_package (TelepathyLoggerQt4 REQUIRED)

# set some default settings
include (KDE4Defaults)

# make some more macros available
include (MacroLibrary)

add_definitions (${KDE4_DEFINITIONS}
                 ${TELEPATHY_LOGGER_QT4_DEFINITIONS}
                 -DQT_NO_CAST_FROM_ASCII
                 -DQT_NO_KEYWORDS
)

include_directories (${KDE4_INCLUDES}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
                     ${TELEPATHY_LOGGER_QT4_INCLUDE_DIRS}
)

configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)

set(KTP_GLOBAL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/global.cpp)

add_subdirectory(KTp)
add_subdirectory(tools)
add_subdirectory(data)
add_subdirectory(tests)

