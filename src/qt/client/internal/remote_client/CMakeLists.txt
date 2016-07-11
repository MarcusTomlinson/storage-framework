set_source_files_properties(${CMAKE_SOURCE_DIR}/data/provider.xml PROPERTIES
  CLASSNAME ProviderInterface
  INCLUDE unity/storage/qt/client/internal/remote_client/dbusmarshal.h)

qt5_add_dbus_interface(generated_files
    ${CMAKE_SOURCE_DIR}/data/provider.xml
    ProviderInterface)

add_library(qt-client-lib-remote OBJECT
    AccountImpl.cpp
    dbusmarshal.cpp
    DownloaderImpl.cpp
    FileImpl.cpp
    FolderImpl.cpp
    HandlerBase.cpp
    ItemImpl.cpp
    RootImpl.cpp
    Runtime_create.cpp
    RuntimeImpl.cpp
    UploaderImpl.cpp
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/remote_client/HandlerBase.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/remote_client/RuntimeImpl.h
    ${generated_files}
)
target_include_directories(qt-client-lib-remote PRIVATE
  ${GLIB_DEPS_INCLUDE_DIRS}
  ${ONLINEACCOUNTS_DEPS_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5DBus_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  )
set_target_properties(qt-client-lib-remote PROPERTIES
  AUTOMOC TRUE
  POSITION_INDEPENDENT_CODE TRUE
)
