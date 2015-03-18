
include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  )

set(model_view_SRCS
    model-view.cpp
)

qt5_wrap_ui(model_view_SRCS model-view.ui)

add_executable(ktp_contact_list_model_view
    ${model_view_SRCS}
    contact-list-model-view-main.cpp
)

target_link_libraries(ktp_contact_list_model_view
  Qt5::Test
  Qt5::Widgets
  KF5::CoreAddons
  KF5::I18n
  KTp::CommonInternals
  KTp::Models
)

if (KF5People_FOUND)

    add_executable(ktp_kpeople_list_model_view
        ${model_view_SRCS}
        kpeople-model-view-main.cpp
    )

    target_link_libraries(ktp_kpeople_list_model_view
    Qt5::Test
    Qt5::Widgets
    KF5::I18n
    KF5::People
    KF5::CoreAddons
    KTp::CommonInternals
    KTp::Models
    )

endif ()


