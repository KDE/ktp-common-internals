add_library(kpeople_chat_plugin MODULE chatlistviewdelegate.cpp kpeople_chat_plugin.cpp)
target_link_libraries(kpeople_chat_plugin ${QT_QTCORE_LIBRARY}
    KF5::PeopleWidgets
    KTp::Logger
    KTp::CommonInternals
    KF5::KDELibs4Support
)

install(TARGETS kpeople_chat_plugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kpeople_chat_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
