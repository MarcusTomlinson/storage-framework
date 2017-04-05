add_executable(provider-utils_test
  utils_test.cpp
)
target_link_libraries(provider-utils_test
  storage-framework-provider-static
  gtest
)
add_test(provider-utils provider-utils_test)
