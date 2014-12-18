add_subdirectory(lib)

set(ktp-proxy-test_LIBS
        Qt5::Test
        ${TELEPATHY_QT5_LIBRARIES}
        ${TELEPATHY_QT5_SERVICE_LIBRARIES}
        ktp-proxy-test-lib
        KTp::OTR
)

message("-- Adding test files:")
file(GLOB TEST_FILES "*.cpp")
set(TEST_NAMES "")

set(CMAKE_AUTOMOC ON)

foreach(file ${TEST_FILES})
    get_filename_component(base_name ${file} NAME_WE)
    get_filename_component(full_name ${file} NAME)

    add_executable(${base_name} ${full_name} )
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
