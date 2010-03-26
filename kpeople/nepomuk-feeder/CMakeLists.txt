project (telepathy_integration_daemon)

# Include our extra FindFoo.cmake files.
set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

find_package (KDE4 REQUIRED)
find_package (Nepomuk REQUIRED)
find_package (TelepathyQt4 REQUIRED)
set(SDO_MIN_VERSION "0.3.60")
find_package(SharedDesktopOntologies ${SDO_MIN_VERSION} REQUIRED)

include (CheckIncludeFiles)
include (KDE4Defaults)
include (MacroLibrary)

# Some magic win32 stuff. FIXME: What does this do?
if (WIN32)
    set (LIB_INSTALL_DIR ${LIB_INSTALL_DIR}
         RUNTIME DESTINATION ${BIN_INSTALL_DIR}
         LIBRARY DESTINATION ${LIB_INSTALL_DIR}
         ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    )
endif (WIN32)

set (KDE4_ICON_DIR
     ${KDE4_INSTALL_DIR}/share/icons
)

include_directories (${KDE4_INCLUDES}
                     ${KDEPIMLIBS_INCLUDE_DIRS}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
                     ${NEPOMUK_INCLUDES}
                     ${CMAKE_CURRENT_BINARY_DIR}/nie
)

set (CMAKE_CXX_FLAGS
     "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}"
)

###################################
# First target - NIE static library

nepomuk_add_ontology_classes (nie_SRCS
                              ONTOLOGIES
                              ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig
                              ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig
                              ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig
                              ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nmo.trig
                              ${CMAKE_CURRENT_SOURCE_DIR}/telepathy.trig
)

kde4_add_library (nie
                  STATIC
                  ${nie_SRCS}
)

target_link_libraries (nie
                       ${QT_QTCORE_LIBRARY}
                       ${NEPOMUK_LIBRARIES}
)

####################################
# Second target - integration daemon

set (telepathy_integration_daemon_SRCS
     telepathyaccountmonitor.cpp
     telepathyaccount.cpp
     telepathycontact.cpp
     main.cpp
)

# Add the ontologies we want to build the vocabulary for.
soprano_add_ontology (pimo_SRCS
                      ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig
                      "PIMO"
                      "Nepomuk::Vocabulary"
                      "trig"
)

soprano_add_ontology (nco_SRCS
                      ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig
                      "NCO"
                      "Nepomuk::Vocabulary"
                      "trig"
)

soprano_add_ontology (telepathy_SRCS
                      ${CMAKE_CURRENT_SOURCE_DIR}/telepathy.trig
                      "Telepathy"
                      "Nepomuk::Vocabulary"
                      "trig"
)

# Build the daemon.
kde4_add_executable (telepathy-integration-daemon
                     ${telepathy_integration_daemon_SRCS}
                     ${pimo_SRCS}
                     ${nco_SRCS}
                     ${telepathy_SRCS}
)

target_link_libraries (telepathy-integration-daemon
                       ${QT_QTCORE_LIBRARY}
                       ${QT_QTDBUS_LIBRARY}
                       ${KDE4_KDECORE_LIBS}
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${NEPOMUK_LIBRARIES}
                       nie
)

# Install the daemon.
install (TARGETS telepathy-integration-daemon
         ${INSTALL_TARGETS_DEFAULT_ARGS}
)


# Install the ontology
macro(INSTALL_ONTOLOGY _name _group)
  set(ONTO_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/ontology/${_group})
  configure_file("${_name}.ontology.in" "${CMAKE_CURRENT_BINARY_DIR}/${_name}.ontology")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${_name}.ontology" DESTINATION ${ONTO_INSTALL_DIR})
  install(FILES "${_name}.trig" DESTINATION ${ONTO_INSTALL_DIR})
endmacro(INSTALL_ONTOLOGY)

install_ontology (telepathy telepathy)
