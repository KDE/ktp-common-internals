include (KTelepathyTestLibMacros)
include (NepomukTestLibMacros)

include_directories(${KTELEPATHYTESTLIB_INCLUDE_DIR})

# Add targets for lcov reports
add_custom_target(lcov-reset lcov --directory ${CMAKE_BINARY_DIR} --zerocounters
                             COMMAND find ${CMAKE_BINARY_DIR} -name '*.gcda' -exec rm -f '{}' ';' || true
                             COMMENT "Cleaning lcov files")

add_custom_target(lcov-check make test || true
                             COMMAND lcov --directory ${CMAKE_BINARY_DIR} --capture --output-file ${CMAKE_BINARY_DIR}/lcov.info &&
                                     mkdir ${CMAKE_BINARY_DIR}/lcov.html || true && genhtml --title Nepomuk-Telepathy-Service
                                     --output-directory ${CMAKE_BINARY_DIR}/lcov.html ${CMAKE_BINARY_DIR}/lcov.info
                             COMMENT "Generating lcov report in file://${CMAKE_BINARY_DIR}/lcov.html/index.html"
                             WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
add_dependencies(lcov-check lcov-reset)

# Setup test environment first
ktelepathy_setup_test_environment()

nepomuk_add_ontology_classes (common_SRCS
                              ONTOLOGIES
                              ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig
                              ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig
                          #   ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig
                          #   ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nmo.trig
                              ${CMAKE_SOURCE_DIR}/telepathy.trig
)

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
# Storage test
kde4_add_executable(storage-test storage-test.cpp ${common_SRCS})
target_link_libraries(storage-test
                      ${KTELEPATHYTESTLIB_LIBRARIES}
                      ${QT_QTTEST_LIBRARY}
                      nepomuktelepathyservice-static)
add_nepomuk_test(StorageTest ${CMAKE_CURRENT_BINARY_DIR}/storage-test)

################################
# Account test
kde4_add_executable(account-test account-test.cpp)
target_link_libraries(account-test
                      ${KTELEPATHYTESTLIB_LIBRARIES}
                      ${QT_QTTEST_LIBRARY}
                      nepomuktelepathyservice-static)
add_ktelepathy_dbus_test(AccountTest ${CMAKE_CURRENT_BINARY_DIR}/account-test)
