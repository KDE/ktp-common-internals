# Try to find the KAccounts UI library
#
# Copyright (c) 2011, Dario Freddi <drf@kde.org>
# Copyright (c) 2014, Martin Klapetek <mklapetek@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

SET (KACCOUNTS_FIND_REQUIRED ${KAccounts_FIND_REQUIRED})
if (KACCOUNTS_INCLUDE_DIR AND KACCOUNTS_LIBRARY)
  # Already in cache, be silent
  set(KACCOUNTS_FIND_QUIETLY TRUE)
endif (KACCOUNTS_INCLUDE_DIR AND KACCOUNTS_LIBRARY)

find_path(KACCOUNTS_INCLUDE_DIR
  NAMES KAccounts/kaccountsuiplugin.h
  PATHS ${CMAKE_INSTALL_FULL_INCLUDEDIR}
)

find_library(KACCOUNTS_LIBRARY NAMES kaccounts)

set(KACCOUNTS_INCLUDE_DIRS ${KACCOUNTS_INCLUDE_DIR})
set(KACCOUNTS_LIBRARIES ${KACCOUNTS_LIBRARY})

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(KAccounts DEFAULT_MSG
                                  KACCOUNTS_LIBRARY
                                  KACCOUNTS_INCLUDE_DIR)

mark_as_advanced(KACCOUNTS_INCLUDE_DIR KACCOUNTS_LIBRARY)
