## cmake macro to test Libgcrypt
# This macro once ran defines:
# LIBGCRYPT_FOUND to TRUE if found
# LIBGCRYPT_INCLUDE_DIR
# LIBGCRYPT_LIBRARIES

# Copyright (c) 2014, Marcin Ziemiński <zieminn@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(LIBGCRYPT_INCLUDE_DIR gcrypt.h)
find_library (LIBGCRYPT_LIBS NAMES gcrypt)

if (LIBGCRYPT_LIBS AND LIBGCRYPT_INCLUDE_DIR)
    message(STATUS "Libgcrypt found: ${LIBGCRYPT_LIBS}")
    set(LIBGCRYPT_FOUND TRUE)
    set(LIBGCRYPT_LIBRARIES ${LIBGCRYPT_LIBS})
elseif(Libgcrypt_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find Libgcrypt")
endif(LIBGCRYPT_LIBS AND LIBGCRYPT_INCLUDE_DIR)
