add_executable(remote-client_test remote-client_test.cpp)
set_target_properties(remote-client_test PROPERTIES AUTOMOC TRUE)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
include_directories(${GLIB_DEPS_INCLUDE_DIRS})

qt5_use_modules(remote-client_test Test)
target_link_libraries(remote-client_test
    storage-framework-qt-client
    Qt5::Network
    Qt5::Test
    ${Boost_LIBRARIES}
    ${GLIB_DEPS_LIBRARIES}
    testutils
    gtest
)
add_test(remote-client remote-client_test)
add_dependencies(remote-client_test qt-client-all-headers)

set(UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} PARENT_SCOPE)
