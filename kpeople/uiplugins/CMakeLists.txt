include_directories(${KPEOPLE_INCLUDES})

kde4_add_plugin(imdetailswidgetplugin imdetailswidget.cpp)
target_link_libraries(imdetailswidgetplugin ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KDE4_KDECORE_LIBRARY}
    ${KDE4_KDEUI_LIBRARY}
    ${KPEOPLE_LIBS}
    ${KDEPIMLIBS_KABC_LIBS}
    kpeoplewidgets
    ktpcommoninternalsprivate
)

install(TARGETS imdetailswidgetplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES imdetailswidgetplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(chatplugin)