set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  )



set(ktp_contact_list_model_view_SRCS
    contact-list-model-view-main.cpp
    model-view.cpp
    roles-proxy-model.cpp
)

kde4_add_ui_files(ktp_contact_list_model_view_SRCS model-view.ui)

kde4_add_executable(ktp_contact_list_model_view
    ${ktp_contact_list_model_view_SRCS}
)

target_link_libraries(ktp_contact_list_model_view
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${TELEPATHY_QT4_LIBRARIES}
  ktpcommoninternalsprivate
  ktpmodelsprivate
)

