project(kaccounts-integration-plugin)

include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set(kaccounts_ktp_plugin_SRCS
    kaccounts-ktp-plugin.cpp
)

add_library(kaccounts_ktp_plugin MODULE ${kaccounts_ktp_plugin_SRCS})

target_link_libraries(kaccounts_ktp_plugin
                      Qt5::Core
                      KF5::ConfigCore
                      KF5::CoreAddons
                      KTp::Logger
                      KAccounts
                      ${ACCOUNTSQT_LIBRARIES}
)

# Install:
install (TARGETS kaccounts_ktp_plugin
         DESTINATION ${PLUGIN_INSTALL_DIR}/kaccounts/daemonplugins
)


