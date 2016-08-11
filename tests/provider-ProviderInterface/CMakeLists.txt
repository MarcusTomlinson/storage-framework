

set_source_files_properties(${CMAKE_SOURCE_DIR}/data/provider.xml PROPERTIES
  CLASSNAME ProviderClient
  INCLUDE unity/storage/qt/client/internal/remote_client/dbusmarshal.h
)
qt5_add_dbus_interface(generated_files
  ${CMAKE_SOURCE_DIR}/data/provider.xml
  ProviderClient
)
set_source_files_properties(${generated_files} PROPERTIES
  GENERATED TRUE
)

add_executable(provider-ProviderInterface_test
  ProviderInterface_test.cpp
  TestProvider.cpp
  ../../src/qt/client/internal/remote_client/dbusmarshal.cpp
  ${generated_files}
)
add_dependencies(provider-ProviderInterface_test storage-framework-provider)
set_target_properties(provider-ProviderInterface_test PROPERTIES
  AUTOMOC TRUE
)
target_link_libraries(provider-ProviderInterface_test
  storage-framework-provider-static
  Qt5::Test
  testutils
  gtest
)
add_test(provider-ProviderInterface provider-ProviderInterface_test)
