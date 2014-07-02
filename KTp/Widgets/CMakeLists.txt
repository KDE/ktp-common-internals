include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_widgets_private_SRCS
    ${KTP_GLOBAL_SOURCES}
    contact-view-widget.cpp
    contact-grid-widget.cpp
    contact-grid-dialog.cpp
    add-contact-dialog.cpp
    join-chat-room-dialog.cpp
    notification-config-dialog.cpp
    accounts-combo-box.cpp
    start-chat-dialog.cpp
    settings-kcm-dialog.cpp
)

set (ktp_widgets_private_HDRS
     contact-view-widget.h
     contact-grid-widget.h
     contact-grid-dialog.h
     add-contact-dialog.h
     join-chat-room-dialog.h
     notification-config-dialog.h
     accounts-combo-box.h
     start-chat-dialog.h
     settings-kcm-dialog.h
)

qt5_wrap_ui (ktp_widgets_private_SRCS
                   add-contact-dialog.ui
                   join-chat-room-dialog.ui
                   start-chat-dialog.ui
)

add_library (ktpwidgetsprivate SHARED
                 ${ktp_widgets_private_SRCS}
)

#Raise SOVERSION for every 0.x cycle
set_target_properties(ktpwidgetsprivate PROPERTIES
                                                VERSION ${KTP_VERSION}
                                                SOVERSION ${KTP_SONUMBER}
                                                DEFINE_SYMBOL MAKE_KTP_LIB)

target_link_libraries (ktpwidgetsprivate
                       
                       ${TELEPATHY_QT5_LIBRARIES}
                       KF5::KDELibs4Support
                       KF5::NotifyConfig
                       KF5::KCMUtils
                       ktpcommoninternalsprivate
                       ktpmodelsprivate
)

install (TARGETS ktpwidgetsprivate
         DESTINATION ${LIB_INSTALL_DIR}
)

install (FILES ${ktp_widgets_private_HDRS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/KTp/Widgets
)
