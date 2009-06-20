project(telepathy_integration_daemon)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

# search packages used by KDE
find_package (KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalAddSubdirectory)
include(CheckIncludeFiles)
find_package (KdepimLibs REQUIRED)

find_package (TelepathyQt4 REQUIRED)

if(WIN32)
    set(LIB_INSTALL_DIR ${LIB_INSTALL_DIR}
                        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
                        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
                        ARCHIVE DESTINATION ${LIB_INSTALL_DIR} )
endif(WIN32)

set(KDE4_ICON_DIR ${KDE4_INSTALL_DIR}/share/icons)

include_directories(
    ${KDE4_INCLUDES}
    ${KDEPIMLIBS_INCLUDE_DIRS}
    ${TELEPATHY_QT4_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )


########### next target ###############

set(telepathy_integration_daemon_SRCS
    main.cpp
)

kde4_add_executable(telepathy-integration-daemon)

target_link_libraries(telepathy-integration-daemon ${KDE4_AKONADI_LIBS}
                                                   ${QT_QTCORE_LIBRARY}
                                                   ${QT_QTDBUS_LIBRARY}
                                                   ${KDE4_KDECORE_LIBS}
                                                   ${TELEPATHY_QT4_LIBRARIES}
)

install(TARGETS telepathy-integration-daemon ${INSTALL_TARGETS_DEFAULT_ARGS})
