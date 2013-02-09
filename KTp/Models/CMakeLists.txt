
include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_models_private_SRCS
    abstract-grouping-proxy-model.cpp
    accounts-list-model.cpp
    accounts-tree-proxy-model.cpp
    contacts-filter-model.cpp
    contacts-list-model.cpp
    contacts-model.cpp
    groups-tree-proxy-model.cpp
    text-channel-watcher-proxy-model.cpp
    rooms-model.cpp
)

set (ktp_models_private_HDRS
    abstract-grouping-proxy-model.h
    accounts-list-model.h
    accounts-tree-proxy-model.h
    contacts-filter-model.h
    contacts-list-model.h
    contacts-model.h
    groups-tree-proxy-model.h
    text-channel-watcher-proxy-model.h
    rooms-model.h
)

kde4_add_library (ktpmodelsprivate SHARED
                 ${ktp_models_private_SRCS}
)

#Raise SOVERSION for every 0.x cycle
set_target_properties(ktpmodelsprivate PROPERTIES
                                                VERSION ${KTP_VERSION}
                                                SOVERSION ${KTP_SONUMBER}
                                                DEFINE_SYMBOL MAKE_KTP_LIB)

target_link_libraries (ktpmodelsprivate
                       ${KDE4_KDECORE_LIBS}
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${KDE4_KDEUI_LIBS}
                       ktpcommoninternalsprivate
)

install (TARGETS ktpmodelsprivate
         DESTINATION ${LIB_INSTALL_DIR}
)

install (FILES ${ktp_models_private_HDRS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/KTp/Models
)

