include_directories(${KPEOPLE_INCLUDES})

kde4_add_plugin(kpeople_chat_plugin chatlistviewdelegate.cpp kpeople_chat_plugin.cpp)
target_link_libraries(kpeople_chat_plugin ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KDECORE_LIBRARY}
    ${KDE4_KDEUI_LIBRARY}
    ${KPEOPLE_LIBS}
    ${KDEPIMLIBS_KABC_LIBS}
    kpeoplewidgets
    ktploggerprivate
    ktpcommoninternalsprivate
)

install(TARGETS kpeople_chat_plugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kpeople_chat_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
