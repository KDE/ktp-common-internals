
include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

set (ktp_models_private_SRCS
    accounts-list-model.cpp
    accounts-filter-model.cpp
    accounts-model-item.cpp #Deprecated
    ../capabilities-hack-private.cpp #HACK, COPY INTO BOTH LIBRARIES UNTIL ContactModels IS REPLACED
    ../service-availability-checker.cpp #
    contacts-list-model.cpp
    contact-model-item.cpp #Deprecated
    contacts-model.cpp #Deprecated
    groups-model-item.cpp #Deprecated
    groups-model.cpp #Deprecated
    proxy-tree-node.cpp #Deprecated
    tree-node.cpp #Deprecated
    flat-model-proxy.cpp #Deprecated
    rooms-model.cpp
)

set (ktp_models_private_HDRS
    accounts-list-model.h
    accounts-filter-model.h
    accounts-model-item.h #Deprecated
    contact-model-item.h #Deprecated
    contacts-model.h #Deprecated
    contacts-list-model.h
    groups-model-item.h #Deprecated
    groups-model.h #Deprecated
    proxy-tree-node.h #Deprecated
    tree-node.h #Deprecated
    flat-model-proxy.h #Deprecated
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

