kcoreaddons_add_plugin(kaccounts_ktp_plugin
    JSON kaccountsplugin.json
    SOURCES kaccounts-ktp-plugin.cpp
    INSTALL_NAMESPACE kaccounts/daemonplugins)

target_link_libraries(kaccounts_ktp_plugin
                      Qt5::Core
                      Qt5::DBus
                      KF5::ConfigCore
                      KF5::CoreAddons
                      KTp::Logger
                      KAccounts
                      ${ACCOUNTSQT_LIBRARIES}
                      ${SIGNONQT_LIBRARIES}
)
