add_executable(local-provider_test local-provider_test.cpp)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}" -DBOOST_THREAD_VERSION=4)

target_link_libraries(local-provider_test
    local-provider-lib
    storage-framework-provider
    storage-framework-qt-client-v2
    Qt5::Test
    ${Boost_LIBRARIES}
    ${GLIB_DEPS_LIBRARIES}
    testutils
    gtest
)
add_test(local-provider local-provider_test)

set(UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} PARENT_SCOPE)
