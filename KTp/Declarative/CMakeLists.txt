
include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_qml_plugin_SRCS
    conversation.cpp
    conversations-model.cpp
    messages-model.cpp
    pinned-contacts-model.cpp
    filtered-pinned-contacts-proxy-model.cpp
    text-area-emojis-handler.cpp
    contact-pin.cpp
    telepathy-manager.cpp
    qml-plugins.cpp
    debug.cpp
    mainlogmodel.cpp
    emojis-model.cpp
)

add_library (ktpqmlplugin SHARED ${ktp_qml_plugin_SRCS})

target_link_libraries (ktpqmlplugin
    Qt5::Qml
    Qt5::Sql
    Qt5::Quick

    KF5::WindowSystem
    KF5::I18n
    KF5::People
    KF5::Emoticons
    KTp::CommonInternals
    KTp::Logger
    KTp::Models
    KTp::Widgets
)

install (TARGETS ktpqmlplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/telepathy)
install (FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/telepathy)
