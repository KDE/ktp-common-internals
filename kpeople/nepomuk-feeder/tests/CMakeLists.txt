include (KTelepathyTestLibMacros)

include_directories(${KTELEPATHYTESTLIB_INCLUDE_DIR})

# Setup test environment first
ktelepathy_setup_test_environment()

# Create our static base test library
kde4_add_library(tid-base-tests STATIC tid-base-test.cpp)
target_link_libraries(tid-base-tests
                      ${KTELEPATHYTESTLIB_LIBRARIES}
                      ${QT_QTTEST_LIBRARY}
                      telepathy-integration-daemon-static)

################################
# Account test
kde4_add_executable(account-test account-test.cpp)
target_link_libraries(account-test tid-base-tests)
add_ktelepathy_nepomuk_test(AccountTest ${CMAKE_CURRENT_BINARY_DIR}/account-test)

################################
# Contact test
kde4_add_executable(contact-test contact-test.cpp)
target_link_libraries(contact-test tid-base-tests)
add_ktelepathy_nepomuk_test(ContactTest ${CMAKE_CURRENT_BINARY_DIR}/contact-test)
