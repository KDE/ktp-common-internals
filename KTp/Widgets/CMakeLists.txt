include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_widgets_private_SRCS
    flat-model-proxy.cpp
    contact-grid-widget.cpp
)

set (ktp_widgets_private_HDRS
#    flat-model-proxy.h
    contact-grid-widget.h
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
                       ktpcommoninternalsprivate
                       ktpmodelsprivate
)

install (TARGETS ktpwidgetsprivate
         DESTINATION ${LIB_INSTALL_DIR}
)

install (FILES ${ktp_widgets_private_HDRS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/KTp/Widgets
)
