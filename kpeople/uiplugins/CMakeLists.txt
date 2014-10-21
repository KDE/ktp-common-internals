include_directories(${KPEOPLE_INCLUDES})

add_library(imdetailswidgetplugin MODULE imdetailswidget.cpp)
target_link_libraries(imdetailswidgetplugin Qt5::Core
    Qt5::Gui
    KF5::KDELibs4Support
    KF5::PeopleWidgets
    KTp::CommonInternals
)

install(TARGETS imdetailswidgetplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES imdetailswidgetplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
