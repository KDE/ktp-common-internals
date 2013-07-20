include_directories(${KPEOPLE_INCLUDES})

set (im_persons_data_source_SRCS
     im-persons-data-source-plugin.cpp
)

kde4_add_plugin (im_persons_data_source_plugin
                 ${im_persons_data_source_SRCS}
)

target_link_libraries (im_persons_data_source_plugin
    ${QT_LIBRARIES}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${NEPOMUK_CORE_LIBRARY}
    ${KPEOPLE_LIBS}
    ktpcommoninternalsprivate
)

# Install:
install (TARGETS im_persons_data_source_plugin
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES im_persons_data_source_plugin.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

