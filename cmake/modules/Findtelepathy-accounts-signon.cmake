## cmake macro to find telepathy-accounts-signon
#
# Copyright (c) 2015, Martin Klapetek <mklapetek@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig)
execute_process(
    COMMAND "${PKG_CONFIG_EXECUTABLE}" --variable=plugindir mission-control-plugins
    OUTPUT_VARIABLE _pkgconfig_invoke_result OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _pkgconfig_failed)

if(EXISTS "${_pkgconfig_invoke_result}/mcp-account-manager-accounts-sso.so") 
    set(telepathy-accounts-signon_FOUND TRUE)
else()
    set(telepathy-accounts-signon_FOUND FALSE)
endif()