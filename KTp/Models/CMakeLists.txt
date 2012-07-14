
include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_models_private_SRCS
    accounts-filter-model.cpp
    accounts-model-item.cpp
    accounts-model.cpp
    capabilities-hack-private.cpp
    contact-model-item.cpp
    groups-model-item.cpp
    groups-model.cpp
    proxy-tree-node.cpp
    tree-node.cpp
    flat-model-proxy.cpp
)

set (ktp_models_private_HDRS
    accounts-filter-model.h
    accounts-model-item.h
    accounts-model.h
    contact-model-item.h
    groups-model-item.h
    groups-model.h
    proxy-tree-node.h
    tree-node.h
    flat-model-proxy.h
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

