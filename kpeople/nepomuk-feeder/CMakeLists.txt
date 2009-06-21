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
find_package(Nepomuk REQUIRED)
include(MacroAddOntology)

# find the pimo.trig file from the kdebase installation
find_file(PIMO_TRIG_SOURCE
  pimo.trig
  PATHS "${DATA_INSTALL_DIR}" "${KDE4_DATA_DIR}" ENV XDG_DATA_DIRS
  PATH_SUFFIXES "nepomuk/ontologies"
  )

if(NOT PIMO_TRIG_SOURCE)
  message(ERROR_FATAL "Could not find pimo.trig PIMO source file. Is kdebase-runtime installed?")
endif(NOT PIMO_TRIG_SOURCE)

# find the nco.trig file from the kdebase installation
find_file(NCO_TRIG_SOURCE
  nco.trig
  PATHS "${DATA_INSTALL_DIR}" "${KDE4_DATA_DIR}" ENV XDG_DATA_DIRS
  PATH_SUFFIXES "nepomuk/ontologies"
  )

if(NOT NCO_TRIG_SOURCE)
  message(ERROR_FATAL "Could not find nco.trig NCO source file. Is kdebase-runtime installed?")
endif(NOT NCO_TRIG_SOURCE)

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
    ${NEPOMUK_INCLUDES}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )


########### next target ###############

set(telepathy_integration_daemon_SRCS
    telepathyaccountmonitor.cpp
    telepathyaccount.cpp
    main.cpp
)

kde4_add_ontology(pimo_SRCS ${PIMO_TRIG_SOURCE} "PIMO" "Nepomuk::Vocabulary" "trig")
kde4_add_ontology(nco_SRCS ${NCO_TRIG_SOURCE} "NCO" "Nepomuk::Vocabulary" "trig")
kde4_add_ontology(tpaccount_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/tpaccount.trig "TPACCOUNT" "Nepomuk::Vocabulary" "trig")

kde4_add_executable(telepathy-integration-daemon ${telepathy_integration_daemon_SRCS}
                                                 ${pimo_SRCS}
                                                 ${nco_SRCS}
                                                 ${tpaccount_SRCS}
)

target_link_libraries(telepathy-integration-daemon ${KDE4_AKONADI_LIBS}
                                                   ${QT_QTCORE_LIBRARY}
                                                   ${QT_QTDBUS_LIBRARY}
                                                   ${KDE4_KDECORE_LIBS}
                                                   ${TELEPATHY_QT4_LIBRARIES}
                                                   ${NEPOMUK_LIBRARIES}
)

install(TARGETS telepathy-integration-daemon ${INSTALL_TARGETS_DEFAULT_ARGS})
