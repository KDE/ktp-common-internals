project(ktp-common-internals)

set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

set (KTP_VERSION "0.4")
# Bump for every 0.x release, or whenever BC changes
set (KTP_SONUMBER 2)

find_package (KDE4 REQUIRED)
find_package (TelepathyQt4 0.9.2.1 REQUIRED)

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

add_subdirectory(KTp)
add_subdirectory(data)
