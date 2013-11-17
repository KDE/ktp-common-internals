include_directories(${KPEOPLE_INCLUDES})

set (ktp_kpeople_plugin_SRCS
     kpeople-actions-plugin.cpp
)

kde4_add_plugin (ktp_kpeople_plugin
                 ${ktp_kpeople_plugin_SRCS}
)

target_link_libraries (ktp_kpeople_plugin
    ${QT_LIBRARIES}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${KPEOPLE_LIBS}
    ${KDE4_KFILE_LIBS}
    ktpcommoninternalsprivate
    ${KDEPIMLIBS_KABC_LIBS}
)

# Install:
install (TARGETS ktp_kpeople_plugin
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES ktp_kpeople_plugin.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)
