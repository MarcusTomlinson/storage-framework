set_source_files_properties(${CMAKE_SOURCE_DIR}/data/provider.xml PROPERTIES CLASSNAME ProviderInterface)
#set_source_files_properties(${CMAKE_SOURCE_DIR}/data/provider.xml PROPERTIES INCLUDE unity/storage/internal/ItemMetadata.h)
set_source_files_properties(${CMAKE_SOURCE_DIR}/data/provider.xml PROPERTIES INCLUDE unity/storage/qt/client/internal/remote_client/dbusmarshal.h)

qt5_add_dbus_interface(generated_files
    ${CMAKE_SOURCE_DIR}/data/provider.xml
    ProviderInterface)

set(QT_CLIENT_LIB_INTERNAL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/AccountImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dbusmarshal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DownloaderImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FileImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FolderImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ItemImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RootImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime_create.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RuntimeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UploaderImpl.cpp
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/remote_client/DownloaderImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/remote_client/RuntimeImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/remote_client/UploaderImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/RuntimeBase.h
    ${generated_files}
)

include_directories(${GLIB_DEPS_INCLUDE_DIRS})
include_directories(${ONLINEACCOUNTS_DEPS_INCLUDE_DIRS})

add_library(storage-framework-qt-client SHARED
    $<TARGET_OBJECTS:qt-client-lib-public>
    ${QT_CLIENT_LIB_INTERNAL_SRC})
qt5_use_modules(storage-framework-qt-client Concurrent Core DBus Network)
set_target_properties(storage-framework-qt-client PROPERTIES AUTOMOC TRUE)
# Commented out for now, until we fill in the implementation.
#set_target_properties(storage-framework-qt-client PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
target_link_libraries(storage-framework-qt-client
    Qt5::Concurrent
    Qt5::Core
    Qt5::DBus
    Qt5::Network
    ${ONLINEACCOUNTS_DEPS_LDFLAGS})
