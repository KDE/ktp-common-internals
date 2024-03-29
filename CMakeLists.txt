cmake_minimum_required (VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "23")
set (RELEASE_SERVICE_VERSION_MINOR "07")
set (RELEASE_SERVICE_VERSION_MICRO "70")

# Bump for every 0.x release, or whenever BC changes
set (KTP_SONUMBER 9) # SO 9 is for 15.08 release
set (KTP_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
set (KTP_MESSAGE_FILTER_FRAMEWORK_VERSION "5")

project(ktp-common-internals VERSION ${KTP_VERSION})

find_package(ECM 1.7.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${ECM_MODULE_PATH}
)

find_package (Qt5 REQUIRED CONFIG COMPONENTS
              DBus
              Qml
              Sql
              Test
              Xml)

find_package (KF5 5.11 REQUIRED COMPONENTS
              CoreAddons
              Notifications
              KIO
              WidgetsAddons
              KCMUtils
              NotifyConfig
              TextEditor
              Wallet
              Config
              WindowSystem
              IconThemes
              People)

set(REQUIRED_TPQT_VERSION 0.9.8)

find_package (TelepathyQt5 ${REQUIRED_TPQT_VERSION} COMPONENTS Core REQUIRED)
find_package (TelepathyQt5 ${REQUIRED_TPQT_VERSION} COMPONENTS Service) #used for the otr-proxy
find_package (TelepathyLoggerQt)

find_package (KAccounts)
find_package (AccountsQt5 1.10 CONFIG)
find_package (SignOnQt5 8.55 CONFIG)
find_package (LibOTR 4.0.0)
find_package (Libgcrypt)
find_package (telepathy-accounts-signon)

if (LIBOTR_FOUND AND LIBGCRYPT_FOUND AND TARGET TelepathyQt5::Service)
    set(OTR_LIBS_FOUND TRUE)
endif ()

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMMarkNonGuiExecutable)
include(CMakePackageConfigHelpers)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(WriteBasicConfigVersionFile)
include(CheckIncludeFiles)
include(FeatureSummary)

set_package_properties(KF5People PROPERTIES DESCRIPTION "Support for KDE Contact Aggregation"
                       URL "https://commits.kde.org/kpeople"
                       TYPE OPTIONAL
                      )
set_package_properties(TelepathyLoggerQt PROPERTIES DESCRIPTION "Qt bindings for TelepathyLogger. This is needed to provide access to chat logs. HIGHLY recommended"
                       URL "https://commits.kde.org/telepathy-logger-qt"
                       TYPE OPTIONAL
                      )
set_package_properties(LibOTR PROPERTIES DESCRIPTION "Required for OTR support" TYPE OPTIONAL)
set_package_properties(Libgcrypt PROPERTIES DESCRIPTION "Required for OTR support" TYPE OPTIONAL)

set_package_properties(telepathy-accounts-signon PROPERTIES
                       PURPOSE "Runtime-only dependency on a Telepathy Mission Control plugin, currently residing at https://gitlab.com/accounts-sso/telepathy-accounts-signon"
                       TYPE RUNTIME
                      )

add_definitions(-DKTP_MESSAGE_FILTER_FRAMEWORK_VERSION=\"${KTP_MESSAGE_FILTER_FRAMEWORK_VERSION}\")
add_definitions(-DTRANSLATION_DOMAIN=\"ktp-common-internals\")
add_definitions (
                 -DQT_NO_CAST_FROM_ASCII
                 -DQT_NO_KEYWORDS
)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

include_directories (
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

if (KF5People_FOUND)
    add_definitions(-DHAVE_KPEOPLE)
    add_subdirectory(kpeople)
endif ()

#If we find KAccounts library, build the plugin for it
if (KAccounts_FOUND AND AccountsQt5_FOUND AND SignOnQt5_FOUND)
    include_directories(${ACCOUNTSQT_INCLUDE_DIRS} ${SIGNONQT_INCLUDE_DIRS})
    add_subdirectory(kaccounts)
endif ()

ecm_setup_version(${KTP_VERSION} VARIABLE_PREFIX KTP
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ktp_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KTpConfigVersion.cmake"
                        SOVERSION ${KTP_SONUMBER})

add_subdirectory(KTp)
add_subdirectory(tools)
add_subdirectory(data)
add_subdirectory(tests)

if (OTR_LIBS_FOUND)
    include_directories (${LIBOTR_INCLUDE_DIR}
                         ${LIBGCRYPT_INCLUDE_DIR}
    )
    add_subdirectory(otr-proxy)
endif (OTR_LIBS_FOUND)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KTp")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KTpConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KTpConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KTpConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KTpConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KTpTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KTpTargets.cmake NAMESPACE KTp:: COMPONENT Devel)

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
