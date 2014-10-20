
include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_qml_plugin_SRCS
    conversation.cpp
    conversations-model.cpp
    hide-window-component.cpp
    messages-model.cpp
    pinned-contacts-model.cpp
    filtered-pinned-contacts-proxy-model.cpp
    contact-pin.cpp
    telepathy-manager.cpp
    qml-plugins.cpp
)

add_library (ktpqmlplugin SHARED ${ktp_qml_plugin_SRCS})

target_link_libraries (ktpqmlplugin
    Qt5::Qml
    
    ${TELEPATHY_QT5_LIBRARIES}
    KF5::KDELibs4Support
    ktpcommoninternalsprivate
    ktploggerprivate
    ktpmodelsprivate
    ktpwidgetsprivate
)

install (TARGETS ktpqmlplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/telepathy)
install (FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/telepathy)
