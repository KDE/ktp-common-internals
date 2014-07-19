project(ktp-proxy)
set(KTP_PROXY_VERSION "0.1.1")

set(KDE_MIN_VERSION "4.13.1")
find_package (KDE4 ${KDE_MIN_VERSION} REQUIRED)
find_package (TelepathyQt4 0.9.1 REQUIRED)
find_package (TelepathyQt4Service 0.9.2.1 REQUIRED)

include_directories (${KDE4_INCLUDES}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
)

include (KDE4Defaults)

add_definitions (${KDE4_DEFINITIONS})
add_definitions (-std=c++11)

configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(KTpProxy)
add_subdirectory(data)
add_subdirectory(test)
