kcoreaddons_add_plugin(kpeople_chat_plugin SOURCES chatlistviewdelegate.cpp kpeople_chat_plugin.cpp JSON "kpeople_chat_plugin.json" INSTALL_NAMESPACE "kpeople/widgets")
target_link_libraries(kpeople_chat_plugin ${QT_QTCORE_LIBRARY}
    KF5::PeopleWidgets
    KF5::CoreAddons
    KF5::I18n
    KTp::Logger
    KTp::CommonInternals
)
