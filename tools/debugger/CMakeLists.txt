project(telepathy-kde-debugger)

find_package(TelepathyQt4 REQUIRED)
find_package(KDE4 REQUIRED)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${TELEPATHY_QT4_INCLUDE_DIR}
    ${KDE4_INCLUDES}
)

set(tp-kde-debugger_SRCS
    main-window.cpp
    debug-messages-model.cpp
    main.cpp
)

kde4_add_ui_files(tp-kde-debugger_SRCS main-window.ui)
kde4_add_executable(telepathy-kde-debugger ${tp-kde-debugger_SRCS})
target_link_libraries(telepathy-kde-debugger ${TELEPATHY_QT4_LIBRARIES} ${KDE4_KDEUI_LIBS})
