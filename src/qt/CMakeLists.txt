add_subdirectory(client)

# Sources for remote client V2 library.
set(QT_CLIENT_LIB_V2_SRC
    Account.cpp
    AccountsJob.cpp
    Item.cpp
    Runtime.cpp
    StorageError.cpp
    internal/AccountImpl.cpp
    internal/AccountsJobImpl.cpp
    internal/ItemImpl.cpp
    internal/RuntimeImpl.cpp
    internal/StorageErrorImpl.cpp
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
