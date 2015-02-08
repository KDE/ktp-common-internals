find_package(Qt5Sql REQUIRED)

add_library (im_persons_data_source_plugin MODULE im-persons-data-source.cpp debug.cpp)

target_link_libraries (im_persons_data_source_plugin
    Qt5::Sql
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::People
    KF5::PeopleBackend
    KTp::CommonInternals
)

# Install:
install (TARGETS im_persons_data_source_plugin
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES im_persons_data_source_plugin.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

