include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_widgets_private_SRCS
    contact-view-widget.cpp
    contact-grid-widget.cpp
    contact-grid-dialog.cpp
    contact-info-dialog.cpp
    add-contact-dialog.cpp
    join-chat-room-dialog.cpp
    notification-config-dialog.cpp
    accounts-combo-box.cpp
    start-chat-dialog.cpp
    settings-kcm-dialog.cpp
    debug.cpp
)

set (ktp_widgets_private_HDRS
     contact-view-widget.h
     contact-grid-widget.h
     contact-grid-dialog.h
     contact-info-dialog.h
     add-contact-dialog.h
     join-chat-room-dialog.h
     notification-config-dialog.h
     accounts-combo-box.h
     start-chat-dialog.h
     settings-kcm-dialog.h
)

ki18n_wrap_ui(ktp_widgets_private_SRCS
    add-contact-dialog.ui
    join-chat-room-dialog.ui
    start-chat-dialog.ui
)

add_library (KTpWidgets
                 ${ktp_widgets_private_SRCS}
)
add_library (KTp::Widgets ALIAS KTpWidgets)

#Raise SOVERSION for every 0.x cycle
set_target_properties(KTpWidgets PROPERTIES
                                    VERSION ${KTP_VERSION}
                                    SOVERSION ${KTP_SONUMBER}
                                    EXPORT_NAME Widgets
                                    DEFINE_SYMBOL MAKE_KTP_LIB)
target_include_directories(KTpWidgets PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

target_link_libraries (KTpWidgets
                    PUBLIC
                       KTp::CommonInternals
                       KF5::KCMUtils # used in settings-kcm-dialog.h
                       Qt5::Widgets
                    PRIVATE
                       KF5::I18n
                       KF5::WidgetsAddons
                       KF5::IconThemes
                       KF5::KIOFileWidgets
                       KF5::Notifications
                       KF5::NotifyConfig
                       KTp::Models
)

install (TARGETS KTpWidgets
         EXPORT KTpTargets
         ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install (FILES ${ktp_widgets_private_HDRS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/KTp/Widgets
)
