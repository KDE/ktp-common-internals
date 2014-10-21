include_directories(${KPEOPLE_INCLUDES})

set (ktp_kpeople_plugin_SRCS
     kpeople-actions-plugin.cpp
)

add_library (ktp_kpeople_plugin MODULE
                 ${ktp_kpeople_plugin_SRCS}
)

target_link_libraries (ktp_kpeople_plugin
    KF5::People
    KTp::CommonInternals
    KF5::KDELibs4Support
)

# Install:
install (TARGETS ktp_kpeople_plugin
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES ktp_kpeople_plugin.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
