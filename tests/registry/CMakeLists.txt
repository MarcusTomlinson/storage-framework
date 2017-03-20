add_executable(registry_test registry_test.cpp)
# for the generated D-Bus skeleton adaptor
target_include_directories(registry_test PRIVATE
  ${CMAKE_BINARY_DIR}/src/registry
)
target_link_libraries(registry_test
  registry-static
  Qt5::DBus
  Qt5::Test
  testutils
  gtest
)
add_test(registry registry_test)

set(UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} PARENT_SCOPE)
