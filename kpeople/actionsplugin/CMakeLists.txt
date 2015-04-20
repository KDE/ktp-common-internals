kcoreaddons_add_plugin (ktp_kpeople_plugin SOURCES kpeople-actions-plugin.cpp JSON "ktp_kpeople_plugin.json" INSTALL_NAMESPACE "kpeople/actions")

target_link_libraries (ktp_kpeople_plugin
    KF5::People
    KF5::PeopleBackend
    KF5::PeopleWidgets
    KF5::I18n
    KF5::Service
    KTp::CommonInternals
)
