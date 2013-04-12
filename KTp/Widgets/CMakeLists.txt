include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_widgets_private_SRCS
    ${KTP_GLOBAL_SOURCES}
    contact-grid-widget.cpp
    contact-grid-dialog.cpp
    add-contact-dialog.cpp
    join-chat-room-dialog.cpp
    notificationconfigdialog.cpp
    accounts-combo-box.cpp
)

set (ktp_widgets_private_HDRS
     contact-grid-widget.h
     contact-grid-dialog.h
     add-contact-dialog.h
     join-chat-room-dialog.h
     notificationconfigdialog.h
     accounts-combo-box.h
)

kde4_add_ui_files (ktp_widgets_private_SRCS
                   add-contact-dialog.ui
                   join-chat-room-dialog.ui
)

kde4_add_library (ktpwidgetsprivate SHARED
                 ${ktp_widgets_private_SRCS}
)

#Raise SOVERSION for every 0.x cycle
set_target_properties(ktpwidgetsprivate PROPERTIES
                                                VERSION ${KTP_VERSION}
                                                SOVERSION ${KTP_SONUMBER}
                                                DEFINE_SYMBOL MAKE_KTP_LIB)

target_link_libraries (ktpwidgetsprivate
                       ${KDE4_KDECORE_LIBS}
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${KDE4_KDEUI_LIBS}
                       ${KDE4_KNOTIFYCONFIG_LIBRARY}
                       ktpcommoninternalsprivate
                       ktpmodelsprivate
)

install (TARGETS ktpwidgetsprivate
         DESTINATION ${LIB_INSTALL_DIR}
)

install (FILES ${ktp_widgets_private_HDRS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/KTp/Widgets
)
