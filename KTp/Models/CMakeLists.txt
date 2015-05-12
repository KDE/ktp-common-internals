set (ktp_models_private_SRCS
    abstract-grouping-proxy-model.cpp
    accounts-list-model.cpp
    accounts-tree-proxy-model.cpp
    contacts-filter-model.cpp
    contacts-list-model.cpp
    contacts-model.cpp
    groups-tree-proxy-model.cpp
    presence-model.cpp
    text-channel-watcher-proxy-model.cpp
    rooms-model.cpp
    debug.cpp
)

set (ktp_models_private_HDRS
    abstract-grouping-proxy-model.h
    accounts-list-model.h
    accounts-tree-proxy-model.h
    contacts-filter-model.h
    contacts-list-model.h
    contacts-model.h
    groups-tree-proxy-model.h
    presence-model.h
    text-channel-watcher-proxy-model.h
    rooms-model.h
    ${CMAKE_CURRENT_BINARY_DIR}/ktpmodels_export.h
)

if (KF5People_FOUND)
    add_definitions(-DHAVE_KPEOPLE)

    set (ktp_models_private_SRCS
         ${ktp_models_private_SRCS}
         kpeopletranslationproxy.cpp
         )

    set (ktp_models_private_LIBS
         ${ktp_models_private_LIBS}
         KF5::People
        )
endif ()

add_library (KTpModels
                 ${ktp_models_private_SRCS}
)
add_library (KTp::Models ALIAS KTpModels)

target_include_directories(KTpModels PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

generate_export_header(KTpModels)

#Raise SOVERSION for every 0.x cycle
set_target_properties(KTpModels PROPERTIES
                                VERSION ${KTP_VERSION}
                                SOVERSION ${KTP_SONUMBER}
                                DEFINE_SYMBOL MAKE_KTP_LIB
                                EXPORT_NAME Models
)

target_link_libraries (KTpModels
    PUBLIC
        KTp::CommonInternals
    PRIVATE
        KF5::I18n
        KF5::ConfigCore
        KF5::WidgetsAddons
        KF5::IconThemes
        ${ktp_models_private_LIBS}
)



install (TARGETS KTpModels
         EXPORT KTpTargets
         ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

install (FILES ${ktp_models_private_HDRS}
         DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KTp/Models
)

