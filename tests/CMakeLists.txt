# We add -g so we get debug info for the gtest stack frames with gdb.
# The warnings are suppressed so we get a noise-free build for gtest and gmock.

set(old_cxx_flags ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wno-old-style-cast -Wno-missing-field-initializers")
find_package(GMock)
set(CMAKE_CXX_FLAGS ${old_cxx_flags})

configure_file(testsetup.h.in testsetup.h @ONLY)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(utils)

set(unit_test_dirs
#    local-client  -TODO: Temporarily disabled due to a "unknown file: Failure" error in CI.
#    remote-client -TODO: Temporarily disabled due to a "Cannot find any online account" error in CI.
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
