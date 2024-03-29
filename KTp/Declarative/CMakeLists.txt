
include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_qml_plugin_SRCS
    conversation.cpp
    conversations-model.cpp
    messages-model.cpp
    pinned-contacts-model.cpp
    filtered-pinned-contacts-proxy-model.cpp
    contact-pin.cpp
    telepathy-manager.cpp
    qml-plugins.cpp
    debug.cpp
    mainlogmodel.cpp
)

add_library (ktpqmlplugin SHARED ${ktp_qml_plugin_SRCS})

target_link_libraries (ktpqmlplugin
    Qt5::Qml
    Qt5::Sql

    KF5::WindowSystem
    KF5::I18n
    KF5::People
    KTp::CommonInternals
    KTp::Logger
    KTp::Models
    KTp::Widgets
)

install (TARGETS ktpqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/telepathy)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/telepathy)
