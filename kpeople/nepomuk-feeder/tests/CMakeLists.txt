include (KTelepathyTestLibMacros)

include_directories(${KTELEPATHYTESTLIB_INCLUDE_DIR})

# Setup test environment first
ktelepathy_setup_test_environment()

# Create our static base test library
#kde4_add_library(tid-base-tests STATIC tid-base-test.cpp)
#target_link_libraries(tid-base-tests
#                      ${KTELEPATHYTESTLIB_LIBRARIES}
#                      ${QT_QTTEST_LIBRARY}
#                      nepomuktelepathyservice-static)

################################
# Controller test
kde4_add_executable(controller-test controller-test.cpp)
target_link_libraries(controller-test
                      ${KTELEPATHYTESTLIB_LIBRARIES}
                      ${QT_QTTEST_LIBRARY}
                      nepomuktelepathyservice-static)
add_ktelepathy_dbus_test(ControllerTest ${CMAKE_CURRENT_BINARY_DIR}/controller-test)

################################
# Contact test
#kde4_add_executable(contact-test contact-test.cpp)
#target_link_libraries(contact-test tid-base-tests)
#add_ktelepathy_dbus_test(ContactTest ${CMAKE_CURRENT_BINARY_DIR}/contact-test)

