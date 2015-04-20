kcoreaddons_add_plugin(imdetailswidgetplugin SOURCES imdetailswidget.cpp JSON "imdetailswidgetplugin.json" INSTALL_NAMESPACE "kpeople/widgets")
target_link_libraries(imdetailswidgetplugin Qt5::Core
    Qt5::Gui
    KF5::PeopleWidgets
    KF5::CoreAddons
    KF5::I18n
    KTp::CommonInternals
)

add_subdirectory(chatplugin)
