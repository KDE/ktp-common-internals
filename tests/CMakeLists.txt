set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  )

set(model_view_SRCS
    model-view.cpp
    roles-proxy-model.cpp
)

kde4_add_ui_files(model_view_SRCS model-view.ui)

###

kde4_add_executable(ktp_contact_list_model_view
    ${model_view_SRCS}
    contact-list-model-view-main.cpp
)

target_link_libraries(ktp_contact_list_model_view
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${TELEPATHY_QT4_LIBRARIES}
  ktpcommoninternalsprivate
  ktpmodelsprivate
)

###

kde4_add_executable(ktp_pinned-contact_list_model_view
    ${model_view_SRCS}
    pinned-contacts-model-view-main.cpp
    ../KTp/Declarative/pinned-contacts-model.cpp
    ../KTp/Declarative/telepathy-text-observer.cpp
    ../KTp/Declarative/conversations-model.cpp
    ../KTp/Declarative/conversation-target.cpp
    ../KTp/Declarative/contact-pin.cpp
    ../KTp/Declarative/messages-model.cpp
    ../KTp/Declarative/conversation-queue-manager.cpp
    ../KTp/Declarative/conversation.cpp
)

target_link_libraries(ktp_pinned-contact_list_model_view
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${TELEPATHY_QT4_LIBRARIES}
  ktpcommoninternalsprivate
  ktpmodelsprivate
)
