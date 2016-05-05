
find_package(Qt5DBus REQUIRED)

qt5_add_dbus_adaptor(generated_files ${CMAKE_SOURCE_DIR}/data/provider.xml unity/storage/provider/internal/ProviderInterface.h unity::storage::provider::internal::ProviderInterface)

set_source_files_properties(bus.xml PROPERTIES CLASSNAME BusInterface)
qt5_add_dbus_interface(generated_files bus.xml businterface)

set(PROVIDER_LIB_SRC
  internal/ProviderInterface.cpp
  ${generated_files}
  )

add_library(storage-framework-provider SHARED
  ${generated_files})
qt5_use_modules(storage-framework-provider DBus)
