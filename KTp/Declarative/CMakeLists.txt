
include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_qml_plugin_SRCS
    ${KTP_GLOBAL_SOURCES}
    contact-list.cpp
    conversation-queue-manager.cpp
    conversation-target.cpp
    conversation.cpp
    conversations-model.cpp
    hide-window-component.cpp
    messages-model.cpp
    telepathy-text-observer.cpp
    pinned-contacts-model.cpp
    filtered-pinned-contacts-proxy-model.cpp
    contact-pin.cpp
    declarative-ktp-actions.cpp

    qml-plugins.cpp
)

kde4_add_library (ktpqmlplugin SHARED ${ktp_qml_plugin_SRCS})

target_link_libraries (ktpqmlplugin
    ${QT_QTSCRIPT_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ktpcommoninternalsprivate
    ktpmodelsprivate
)

install (TARGETS ktpqmlplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/telepathy)
install (FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/telepathy)
