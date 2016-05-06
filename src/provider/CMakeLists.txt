
find_package(Qt5DBus REQUIRED)
find_package(Boost COMPONENTS thread REQUIRED)


qt5_add_dbus_adaptor(generated_files ${CMAKE_SOURCE_DIR}/data/provider.xml unity/storage/provider/internal/ProviderInterface.h unity::storage::provider::internal::ProviderInterface)

set_source_files_properties(bus.xml PROPERTIES CLASSNAME BusInterface)
qt5_add_dbus_interface(generated_files bus.xml businterface)

add_library(storage-framework-provider SHARED
  internal/ProviderInterface.cpp
  internal/Handler.cpp
  ${generated_files})

set_target_properties(storage-framework-provider PROPERTIES AUTOMOC TRUE)
qt5_use_modules(storage-framework-provider Core DBus)
target_link_libraries(storage-framework-provider
  Qt5::Core
  Qt5::DBus
  ${Boost_LIBRARIES}
)
