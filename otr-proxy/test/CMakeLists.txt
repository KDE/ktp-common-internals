add_subdirectory(lib)

set(ktp-proxy-test_LIBS
        ${KDE4_KDECORE_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KIO_LIBS}
        ${QT_QTTEST_LIBRARY}
        ${TELEPATHY_QT4_LIBRARIES}
        ${TELEPATHY_QT4_SERVICE_LIBRARIES}
        ktp-proxy-test-lib
        ktpotrprivate
)

message("-- Adding test files:")
file(GLOB TEST_FILES "*.cpp")
set(TEST_NAMES "")

set(CMAKE_AUTOMOC ON)

foreach(file ${TEST_FILES})
    get_filename_component(base_name ${file} NAME_WE)
    get_filename_component(full_name ${file} NAME)

    kde4_add_unit_test(${base_name} TESTNAME ktp-proxy-${base_name} ${full_name})
    target_link_libraries (${base_name} ${ktp-proxy-test_LIBS})

    LIST(APPEND TEST_NAMES ${base_name})
endforeach()

message("--   Tests found:")
foreach(test ${TEST_NAMES})
    message("--    " ${test})
endforeach()

add_custom_target(copy_resources)
add_custom_command(TARGET copy_resources PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources)

ADD_CUSTOM_TARGET(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS ${TEST_NAMES} copy_resources)
