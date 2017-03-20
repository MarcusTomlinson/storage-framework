find_package(GMock)

configure_file(testsetup.h.in testsetup.h @ONLY)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(utils)

set(unit_test_dirs
    client
    local-provider
    provider-AccountData
    provider-DBusPeerCache
    provider-ProviderInterface
    provider-Server
    registry
)

set(slow_test_dirs
)

set(UNIT_TEST_TARGETS "")

foreach(dir ${unit_test_dirs})
    add_subdirectory(${dir})
    list(APPEND UNIT_TEST_TARGETS "${dir}_test")
endforeach()

if (${slowtests})
    foreach(dir ${slow_test_dirs})
        add_subdirectory(${dir})
        list(APPEND UNIT_TEST_TARGETS "${dir}_test")
    endforeach()
    add_subdirectory(headers)
endif()

set(UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} PARENT_SCOPE)

if (${slowtests})
    add_subdirectory(copyright)
endif()
add_subdirectory(whitespace)
add_subdirectory(debian-version)
