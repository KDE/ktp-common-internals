project(ktp-debugger)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(ktp-debugger_SRCS
    debug-message-view.cpp
    telepathy-process.cpp
    main-window.cpp
    main.cpp
)

ki18n_wrap_ui(ktp-debugger_SRCS main-window.ui)
add_executable(ktp-debugger ${ktp-debugger_SRCS})
target_link_libraries(ktp-debugger
    ${TELEPATHY_QT5_LIBRARIES}
    KF5::IconThemes
    KF5::TextEditor
    KF5::I18n
    KF5::IconThemes
    KF5::XmlGui
)
target_include_directories(ktp-debugger PUBLIC ${TELEPATHY_QT5_INCLUDE_DIR}) # TODO: Remove when TelepathyQt exports include paths properly

install(TARGETS ktp-debugger DESTINATION ${BIN_INSTALL_DIR})
install(FILES ktpdebugoutput.xml DESTINATION ${KDE_INSTALL_DATADIR}/katepart5/syntax)
