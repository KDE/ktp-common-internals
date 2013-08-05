if (TELEPATHY_LOGGER_QT4_FOUND)
    add_subdirectory(tplogger)
endif(TELEPATHY_LOGGER_QT4_FOUND)

if (WIN32)
    add_subdirectory(winlogger)
endif(WIN32)
