include_directories(${KPEOPLE_INCLUDES})

add_library(imdetailswidgetplugin MODULE imdetailswidget.cpp)
target_link_libraries(imdetailswidgetplugin Qt5::Core
    Qt5::Gui
    ${TELEPATHY_QT5_LIBRARIES}
    ${KDE4_KDECORE_LIBRARY}
    ${KDE4_KDEUI_LIBRARY}
    ${KPEOPLE_LIBS}
    ${KDEPIMLIBS_KABC_LIBS}
    kpeoplewidgets
    ktpcommoninternalsprivate
)

install(TARGETS imdetailswidgetplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES imdetailswidgetplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
