set(ktp-proxy-test-lib_SRCS
    test-session.cpp
    test-config.cpp
)

add_library(ktp-proxy-test-lib STATIC ${ktp-proxy-test-lib_SRCS})
target_link_libraries(ktp-proxy-test-lib ktp-proxy-lib)
