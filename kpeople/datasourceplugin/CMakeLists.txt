find_package(Qt5Sql REQUIRED)

kcoreaddons_add_plugin (im_persons_data_source_plugin SOURCES im-persons-data-source.cpp debug.cpp JSON "im_persons_data_source_plugin.json" INSTALL_NAMESPACE "kpeople/datasource")

target_link_libraries (im_persons_data_source_plugin
    Qt5::Sql
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::People
    KF5::PeopleBackend
    KTp::CommonInternals
)
