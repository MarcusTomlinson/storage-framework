
add_definitions(
  -DBOOST_THREAD_VERSION=4
  -DBOOST_THREAD_PROVIDES_EXECUTORS
  ${APPARMOR_DEPS_CFLAGS}
  ${ONLINEACCOUNTS_DEPS_CFLAGS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

qt5_add_dbus_adaptor(generated_files ${CMAKE_SOURCE_DIR}/data/provider.xml unity/storage/provider/internal/ProviderInterface.h unity::storage::provider::internal::ProviderInterface)

set_source_files_properties(bus.xml PROPERTIES CLASSNAME BusInterface)
qt5_add_dbus_interface(generated_files bus.xml businterface)

add_library(storage-framework-provider SHARED
  ProviderBase.cpp
  Server.cpp
  TempfileUploadJob.cpp
  UploadJob.cpp
  internal/AccountData.cpp
  internal/CredentialsCache.cpp
  internal/Handler.cpp
  internal/MainLoopExecutor.cpp
  internal/PendingJobs.cpp
  internal/ProviderInterface.cpp
  internal/ServerImpl.cpp
  internal/TempfileUploadJobImpl.cpp
  internal/UploadJobImpl.cpp
  internal/dbusmarshal.cpp
  ${CMAKE_SOURCE_DIR}/include/unity/storage/provider/internal/AccountData.h
  ${CMAKE_SOURCE_DIR}/include/unity/storage/provider/internal/Handler.h
  ${CMAKE_SOURCE_DIR}/include/unity/storage/provider/internal/MainLoopExecutor.h
  ${CMAKE_SOURCE_DIR}/include/unity/storage/provider/internal/PendingJobs.h
  ${CMAKE_SOURCE_DIR}/include/unity/storage/provider/internal/ProviderInterface.h
  ${CMAKE_SOURCE_DIR}/include/unity/storage/provider/internal/ServerImpl.h
  ${CMAKE_SOURCE_DIR}/include/unity/storage/provider/internal/TempfileUploadJobImpl.h
  ${CMAKE_SOURCE_DIR}/include/unity/storage/provider/internal/UploadJobImpl.h
  ${generated_files})

set_target_properties(storage-framework-provider PROPERTIES AUTOMOC TRUE)
qt5_use_modules(storage-framework-provider Core DBus Network)
target_link_libraries(storage-framework-provider
  Qt5::Core
  Qt5::DBus
  Qt5::Network
  ${Boost_LIBRARIES}
  ${APPARMOR_DEPS_LDFLAGS}
  ${ONLINEACCOUNTS_DEPS_LDFLAGS}
)
