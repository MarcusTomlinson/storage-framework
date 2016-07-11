add_library(qt-client-lib-public OBJECT
    Account.cpp
    Downloader.cpp
    Exceptions.cpp
    File.cpp
    Folder.cpp
    Item.cpp
    Root.cpp
    Runtime.cpp
    Uploader.cpp
    internal/AccountBase.cpp
    internal/DownloaderBase.cpp
    internal/FileBase.cpp
    internal/FolderBase.cpp
    internal/ItemBase.cpp
    internal/RootBase.cpp
    internal/RuntimeBase.cpp
    internal/UploaderBase.cpp
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/DownloaderBase.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/UploaderBase.h
)
target_include_directories(qt-client-lib-public PRIVATE
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS})
set_target_properties(qt-client-lib-public PROPERTIES
  AUTOMOC TRUE
  POSITION_INDEPENDENT_CODE TRUE
)

add_subdirectory(internal)

# Build the local loopback client library
add_library(storage-framework-qt-local-client SHARED
  $<TARGET_OBJECTS:qt-client-lib-public>
  $<TARGET_OBJECTS:qt-client-lib-local>)
qt5_use_modules(storage-framework-qt-local-client Concurrent Core Network)
set_target_properties(storage-framework-qt-local-client PROPERTIES
  AUTOMOC TRUE
  LINK_FLAGS "-Wl,--no-undefined"
  OUTPUT_NAME "storage-framework-qt-local-client-${STORAGE_FRAMEWORK_API_VERSION}"
  SOVERSION ${STORAGE_CLIENT_SOVERSION}
  VERSION ${STORAGE_CLIENT_LIBVERSION}
)
target_link_libraries(storage-framework-qt-local-client
    Qt5::Concurrent
    Qt5::Core
    Qt5::Network
    ${Boost_LIBRARIES}
    ${GLIB_DEPS_LIBRARIES})

install(
  TARGETS storage-framework-qt-local-client
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

configure_file(
  storage-framework-qt-local-client.pc.in
  storage-framework-qt-local-client-${STORAGE_FRAMEWORK_API_VERSION}.pc
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/storage-framework-qt-local-client-${STORAGE_FRAMEWORK_API_VERSION}.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Build the remote client library

add_library(storage-framework-qt-client SHARED
    $<TARGET_OBJECTS:qt-client-lib-public>
    $<TARGET_OBJECTS:qt-client-lib-remote>)
qt5_use_modules(storage-framework-qt-client Core DBus Network)
set_target_properties(storage-framework-qt-client PROPERTIES
  AUTOMOC TRUE
  LINK_FLAGS "-Wl,--no-undefined"
  OUTPUT_NAME "storage-framework-qt-client-${STORAGE_FRAMEWORK_API_VERSION}"
  SOVERSION ${STORAGE_CLIENT_SOVERSION}
  VERSION ${STORAGE_CLIENT_LIBVERSION}
)
target_link_libraries(storage-framework-qt-client
    Qt5::Concurrent
    Qt5::Core
    Qt5::DBus
    Qt5::Network
    ${Boost_LIBRARIES}
    ${ONLINEACCOUNTS_DEPS_LDFLAGS})

install(
  TARGETS storage-framework-qt-client
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

configure_file(
  storage-framework-qt-client.pc.in
  storage-framework-qt-client-${STORAGE_FRAMEWORK_API_VERSION}.pc
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/storage-framework-qt-client-${STORAGE_FRAMEWORK_API_VERSION}.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
