add_executable(provider-Server_test
  Server_test.cpp
  ../provider-ProviderInterface/TestProvider.cpp
)
set_target_properties(provider-Server_test PROPERTIES
  AUTOMOC TRUE
)
target_link_libraries(provider-Server_test
  storage-framework-provider-static
  Qt5::Test
  testutils
  gtest
)
add_test(provider-Server provider-Server_test)
