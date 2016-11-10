add_subdirectory(client)

set_source_files_properties(${CMAKE_SOURCE_DIR}/data/provider.xml PROPERTIES
    CLASSNAME ProviderInterface
    INCLUDE unity/storage/internal/dbusmarshal.h
)
qt5_add_dbus_interface(generated_files
    ${CMAKE_SOURCE_DIR}/data/provider.xml
    ProviderInterface
)

set_source_files_properties(${CMAKE_SOURCE_DIR}/data/registry.xml PROPERTIES
    CLASSNAME RegistryInterface
    INCLUDE unity/storage/internal/AccountDetails.h
)
qt5_add_dbus_interface(generated_files
    ${CMAKE_SOURCE_DIR}/data/registry.xml
    RegistryInterface
)

set_source_files_properties(${generated_files} dbusmarshal.cpp PROPERTIES
    COMPILE_FLAGS "-Wno-ctor-dtor-privacy -Wno-missing-field-initializers"
    GENERATED TRUE
)

# Sources for remote client V2 library.
set(QT_CLIENT_LIB_V2_SRC
    Account.cpp
    AccountsJob.cpp
    Downloader.cpp
    Item.cpp
    ItemJob.cpp
    ItemListJob.cpp
    Runtime.cpp
    StorageError.cpp
    Uploader.cpp
    VoidJob.cpp
    internal/AccountImpl.cpp
    internal/AccountsJobImpl.cpp
    internal/DownloaderImpl.cpp
    internal/HandlerBase.cpp
    internal/ItemImpl.cpp
    internal/ItemJobImpl.cpp
    internal/ItemListJobImpl.cpp
    internal/ListJobImplBase.cpp
    internal/MultiItemJobImpl.cpp
    internal/MultiItemListJobImpl.cpp
    internal/RuntimeImpl.cpp
    internal/StorageErrorImpl.cpp
    internal/unmarshal_error.cpp
    internal/validate.cpp
    internal/UploaderImpl.cpp
    internal/VoidJobImpl.cpp
    ${generated_files}
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/Account.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/AccountsJob.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/Downloader.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/Item.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/ItemJob.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/ItemListJob.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/Runtime.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/Uploader.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/VoidJob.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/internal/DownloaderImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/internal/AccountsJobImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/internal/HandlerBase.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/internal/ItemJobImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/internal/ItemListJobImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/internal/ListJobImplBase.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/internal/MultiItemJobImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/internal/MultiItemListJobImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/internal/UploaderImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/internal/VoidJobImpl.h
)

add_library(storage-framework-qt-client-v2 SHARED
    ${QT_CLIENT_LIB_V2_SRC}
    ${generated_files}
)
target_include_directories(storage-framework-qt-client-v2 PRIVATE
    ${Qt5DBus_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${ONLINEACCOUNTS_DEPS_INCLUDE_DIRS}
)
set_target_properties(storage-framework-qt-client-v2 PROPERTIES
    AUTOMOC TRUE
    LINK_FLAGS "-Wl,--no-undefined"
    OUTPUT_NAME "storage-framework-qt-client-${SF_CLIENT_API_VERSION}"
    SOVERSION ${SF_CLIENT_SOVERSION}
    VERSION ${SF_CLIENT_LIBVERSION}
)
target_link_libraries(storage-framework-qt-client-v2
    storage-framework-common-internal
    Qt5::Core
    Qt5::DBus
    Qt5::Network
    ${ONLINEACCOUNTS_DEPS_LDFLAGS}
)
install(
    TARGETS storage-framework-qt-client-v2
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

configure_file(
    storage-framework-qt-client.pc.in
    storage-framework-qt-client-${SF_CLIENT_API_VERSION}.pc
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/storage-framework-qt-client-${SF_CLIENT_API_VERSION}.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
