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

qt5_wrap_ui(model_view_SRCS model-view.ui)

###

add_executable(ktp_contact_list_model_view
    ${model_view_SRCS}
    contact-list-model-view-main.cpp
)

target_link_libraries(ktp_contact_list_model_view
  Qt5::Test
  
  ${KDE4_KDEUI_LIBS}
  ${TELEPATHY_QT5_LIBRARIES}
  ktpcommoninternalsprivate
  ktpmodelsprivate
)

if (KF5People_FOUND)

include_directories(${KPEOPLE_INCLUDES})


add_executable(ktp_kpeople_list_model_view
    ${model_view_SRCS}
    kpeople-model-view-main.cpp
)

target_link_libraries(ktp_kpeople_list_model_view
  Qt5::Test
  
  KF5::KDELibs4Support
  ${TELEPATHY_QT5_LIBRARIES}
  KF5::People
  ktpcommoninternalsprivate
  ktpmodelsprivate
)

endif ()


