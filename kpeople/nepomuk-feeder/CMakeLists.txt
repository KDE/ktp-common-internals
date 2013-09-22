project (telepathy_integration_daemon)

# Include our extra FindFoo.cmake files.
set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(KDE_MIN_VERSION "4.8.0")
find_package (KDE4 ${KDE_MIN_VERSION} REQUIRED)
find_package (Soprano REQUIRED)
find_package (NepomukCore REQUIRED)
find_package (TelepathyQt4 0.8.9 REQUIRED)
set(SDO_MIN_VERSION "0.6.50")
find_package(SharedDesktopOntologies ${SDO_MIN_VERSION} REQUIRED)

include (CheckIncludeFiles)
include (KDE4Defaults)
include (MacroLibrary)
include (SopranoAddOntology)

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
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${KDEPIMLIBS_INCLUDE_DIRS}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
                     ${NEPOMUK_INCLUDES}
                     ${CMAKE_BINARY_DIR}
)

set (CMAKE_CXX_FLAGS
     "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}"
)

####################################
# First target - nepomuktelepathyservice static library

set (nepomuktelepathyservice_static_SRCS
     capabilities-hack-private.cpp
     controller.cpp
     account.cpp
     abstract-storage.cpp
     nepomuk-storage.cpp
     test-backdoors.cpp
)

soprano_add_ontology (nepomuktelepathyservice_static_SRCS
                      ${CMAKE_SOURCE_DIR}/kpeople/nepomuk-feeder/telepathy.trig
                      "Telepathy"
                      "Nepomuk2::Vocabulary"
                      "trig"
)

kde4_add_library (nepomuktelepathyservice-static
                  STATIC
                  ${nepomuktelepathyservice_static_SRCS}
)

target_link_libraries (nepomuktelepathyservice-static
                       ${QT_QTCORE_LIBRARY}
                       ${QT_QTDBUS_LIBRARY}
                       ${KDE4_KDECORE_LIBS}
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${SOPRANO_LIBRARIES}
                       ${NEPOMUK_CORE_LIBRARY}
)

set_property(TARGET nepomuktelepathyservice-static PROPERTY COMPILE_FLAGS -fPIC)

####################################
# Second target - nepomuktelepatyservice

set (nepomuktelepathyservice_SRCS
     service.cpp
)

# Build the service.
kde4_add_plugin(nepomuktelepathyservice
                ${nepomuktelepathyservice_SRCS}
)

target_link_libraries (nepomuktelepathyservice
                       nepomuktelepathyservice-static
)

# Install the service.
install (TARGETS nepomuktelepathyservice
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES nepomuktelepathyservice.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

# Install the ontology
macro(INSTALL_ONTOLOGY _name _group)
  set(ONTO_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/ontology/${_group})
  configure_file("${_name}.ontology.in" "${CMAKE_CURRENT_BINARY_DIR}/${_name}.ontology")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${_name}.ontology" DESTINATION ${ONTO_INSTALL_DIR})
  install(FILES "${_name}.trig" DESTINATION ${ONTO_INSTALL_DIR})
endmacro(INSTALL_ONTOLOGY)

install_ontology (telepathy telepathy)

#find_package (KTelepathyTestLib)

#if (KTelepathyTestLib_DIR)
#    add_subdirectory(tests)
#endif (KTelepathyTestLib_DIR)
