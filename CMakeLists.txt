project(ktp-common-internals)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR}
)
cmake_policy(SET CMP0002 OLD)

set (KTP_VERSION "0.8.0")
# Bump for every 0.x release, or whenever BC changes
set (KTP_SONUMBER 9)
set (KTP_MESSAGE_FILTER_FRAMEWORK_VERSION "5")

find_package (Qt5 REQUIRED CONFIG COMPONENTS Qml Test)
find_package (KF5 REQUIRED CONFIG COMPONENTS KCMUtils NotifyConfig TextEditor Wallet KDE4Support)
find_package (TelepathyQt5 0.9.2.1 REQUIRED)
find_package (TelepathyLoggerQt5 QUIET)
find_package (KPeople 0.3 QUIET)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMPackageConfigHelpers)
include(ECMMarkNonGuiExecutable)
include(ECMPackageConfigHelpers)
include(ECMInstallIcons)
include(CMakePackageConfigHelpers)
include(WriteBasicConfigVersionFile)
include(CheckIncludeFiles)
include(FeatureSummary)

set_package_properties(KPeople PROPERTIES DESCRIPTION "Support for KDE Contact Aggregation"
                       URL "https://projects.kde.org/libkpeople"
                       TYPE OPTIONAL
                      )
set_package_properties(TelepathyLoggerQt5 PROPERTIES DESCRIPTION "Qt bindings for TelepathyLogger. This is needed to provide access to chat logs. HIGHLY recommended"
                       URL "http://projects.kde.org/telepathy-logger-qt"
                       TYPE OPTIONAL
                      )


add_definitions (
                 -DQT_NO_CAST_FROM_ASCII
                 -DQT_NO_KEYWORDS
)

include_directories (
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${TELEPATHY_QT5_INCLUDE_DIR}
)

if (TELEPATHY_LOGGER_QT5_FOUND)
    add_definitions(${TELEPATHY_LOGGER_QT5_DEFINITIONS}
                    -DHAVE_TPLOGGERQT)
    include_directories(${TELEPATHY_LOGGER_QT5_INCLUDE_DIRS})
endif ()

if (KPEOPLE_FOUND AND KDEPIMLIBS_FOUND)
    include_directories (${KDEPIMLIBS_INCLUDE_DIRS})
    add_definitions(-DHAVE_KPEOPLE)
    add_subdirectory(kpeople)
endif ()


configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)

set(KTP_GLOBAL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/global.cpp)

add_subdirectory(KTp)
add_subdirectory(tools)
add_subdirectory(data)
add_subdirectory(tests)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
