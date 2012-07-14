project(ktp-debugger)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${TELEPATHY_QT4_INCLUDE_DIR}
    ${KDE4_INCLUDES}
)

set(ktp-debugger_SRCS
    main-window.cpp
    debug-messages-model.cpp
    main.cpp
)

kde4_add_ui_files(ktp-debugger_SRCS main-window.ui)
kde4_add_executable(ktp-debugger ${ktp-debugger_SRCS})
target_link_libraries(ktp-debugger ${TELEPATHY_QT4_LIBRARIES} ${KDE4_KDEUI_LIBS})

install(TARGETS ktp-debugger DESTINATION ${BIN_INSTALL_DIR})
