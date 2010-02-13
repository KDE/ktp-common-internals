project (telepathy_integration_daemon)

# Include our extra FindFoo.cmake files.
set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

find_package (KDE4 REQUIRED)
find_package (Nepomuk REQUIRED)
find_package (TelepathyQt4 REQUIRED)

include (CheckIncludeFiles)
include (KDE4Defaults)
include (MacroLibrary)
include (MacroOptionalAddSubdirectory)
include (NepomukAddOntologyClasses)

# Find pimo.trig, which is installed already by kdebase.
find_file (PIMO_TRIG_SOURCE
           pimo.trig
           PATHS "${DATA_INSTALL_DIR}" "${KDE4_DATA_DIR}"
           ENV XDG_DATA_DIRS
           PATH_SUFFIXES "ontology/pimo"
)

# We depend on pimo.trig, so fail horribly if it's not found.
if (NOT PIMO_TRIG_SOURCE)
    message (ERROR_FATAL
             "Could not find pimo.trig PIMO source file. Is kdebase-runtime installed?"
    )
endif (NOT PIMO_TRIG_SOURCE)

# Find nco.trig, which is installed already by kdebase.
find_file (NCO_TRIG_SOURCE
           nco.trig
           PATHS "${DATA_INSTALL_DIR}" "${KDE4_DATA_DIR}"
           ENV XDG_DATA_DIRS
           PATH_SUFFIXES "ontology/nie"
)

# We depend on nco.trig, so fail horribly if it's not found.
if (NOT NCO_TRIG_SOURCE)
    message (ERROR_FATAL
             "Could not find nco.trig NCO source file. Is kdebase-runtime installed?"
    )
endif (NOT NCO_TRIG_SOURCE)

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


# Build-system setup is now complete. Add any subdirectories we want to build too.
add_subdirectory (nie)

##### Main build target. #####

set (telepathy_integration_daemon_SRCS
     telepathyaccountmonitor.cpp
     telepathyaccount.cpp
     telepathycontact.cpp
     main.cpp
)

# Add the ontologys we want to build the vocabulary for.
soprano_add_ontology (pimo_SRCS
                      ${PIMO_TRIG_SOURCE}
                      "PIMO"
                      "Nepomuk::Vocabulary"
                      "trig"
)

soprano_add_ontology (nco_SRCS
                      ${NCO_TRIG_SOURCE}
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

