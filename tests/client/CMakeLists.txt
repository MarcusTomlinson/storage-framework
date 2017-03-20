add_executable(client_test client_test.cpp MockProvider.cpp)
set_target_properties(client_test PROPERTIES AUTOMOC TRUE)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}" -DBOOST_THREAD_VERSION=4)
include_directories(${GLIB_DEPS_INCLUDE_DIRS})

target_link_libraries(client_test
    storage-framework-provider
    storage-framework-qt-client
    Qt5::Network
    Qt5::Test
    ${Boost_LIBRARIES}
    ${GLIB_DEPS_LIBRARIES}
    testutils
    gtest
)
add_test(client client_test)
add_dependencies(client_test qt-client-all-headers storage-framework-registry)

set(UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} PARENT_SCOPE)
