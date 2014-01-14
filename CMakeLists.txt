project(ktp-common-internals)

cmake_minimum_required (VERSION 2.6.3 FATAL_ERROR)

set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)
cmake_policy(SET CMP0002 OLD)

set (KTP_VERSION "0.7.1")
# Bump for every 0.x release, or whenever BC changes
set (KTP_SONUMBER 6)
set (KTP_MESSAGE_FILTER_FRAMEWORK_VERSION "4")

find_package (KDE4 REQUIRED)
find_package (TelepathyQt4 0.9.2.1 REQUIRED)
find_package (TelepathyLoggerQt4 QUIET)
find_package (KPeople QUIET)

# set some default settings
include (KDE4Defaults)

# make some more macros available
include (MacroLibrary)

macro_log_feature(KPEOPLE_FOUND "KPeople" "Support for KDE Contact Aggregation" "https://projects.kde.org/libkpeople" FALSE "" "")
macro_log_feature(TELEPATHY_LOGGER_QT4_FOUND "TelepatyLoggerQt4" "Qt bindings for TelepathyLogger. This is needed to provide access to chat logs. HIGHLY recommended" "http://projects.kde.org/telepathy-logger-qt" FALSE "" "")
macro_display_feature_log()


add_definitions (${KDE4_DEFINITIONS}
                 -DQT_NO_CAST_FROM_ASCII
                 -DQT_NO_KEYWORDS
)

include_directories (${KDE4_INCLUDES}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
)

if (TELEPATHY_LOGGER_QT4_FOUND)
    add_definitions(${TELEPATHY_LOGGER_QT4_DEFINITIONS}
                    -DHAVE_TPLOGGERQT)
    include_directories(${TELEPATHY_LOGGER_QT4_INCLUDE_DIRS})
endif (TELEPATHY_LOGGER_QT4_FOUND)

if (KPEOPLE_FOUND)
    add_definitions(-DHAVE_KPEOPLE)
    add_subdirectory(kpeople)
endif (KPEOPLE_FOUND)


configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)

set(KTP_GLOBAL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/global.cpp)

add_subdirectory(KTp)
add_subdirectory(tools)
add_subdirectory(data)
add_subdirectory(tests)

