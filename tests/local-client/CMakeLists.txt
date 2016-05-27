add_executable(local-client_test local-client_test.cpp)
set_target_properties(local-client_test PROPERTIES AUTOMOC TRUE)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
include_directories(${GLIB_DEPS_INCLUDE_DIRS})

qt5_use_modules(local-client_test Test)
target_link_libraries(local-client_test
    storage-framework-qt-local-client
    Qt5::Network
    Qt5::Test
    ${Boost_LIBRARIES}
    ${GLIB_DEPS_LIBRARIES}
    gtest
)
add_test(local-client local-client_test)
list(APPEND UNIT_TEST_TARGETS "local-client")

set(UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} PARENT_SCOPE)
