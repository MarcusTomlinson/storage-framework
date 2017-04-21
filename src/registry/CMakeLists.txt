set(TARGET "storage-framework-registry")

qt5_add_dbus_adaptor(adaptor_files
    ${CMAKE_SOURCE_DIR}/data/registry.xml
    unity/storage/registry/internal/RegistryAdaptor.h
    unity::storage::registry::internal::RegistryAdaptor
)

set_source_files_properties(${adaptor_files} PROPERTIES
    COMPILE_FLAGS "-Wno-ctor-dtor-privacy -Wno-missing-field-initializers"
    GENERATED TRUE
)

add_library(registry-static STATIC
    internal/ListAccountsHandler.cpp
    internal/qdbus-last-error-msg.cpp
    internal/RegistryAdaptor.cpp
    ${CMAKE_SOURCE_DIR}/include/unity/storage/registry/internal/ListAccountsHandler.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/registry/internal/RegistryAdaptor.h
    ${adaptor_files})

set_target_properties(registry-static PROPERTIES AUTOMOC TRUE)

target_include_directories(registry-static PUBLIC
    ${GLIB_DEPS_INCLUDE_DIRS}
    ${ONLINEACCOUNTS_DEPS_INCLUDE_DIRS}
)

target_link_libraries(registry-static PUBLIC
    storage-framework-common-internal
    Qt5::Core
    Qt5::DBus
    ${GLIB_DEPS_LDFLAGS}
    ${ONLINEACCOUNTS_DEPS_LDFLAGS}
)

add_executable(${TARGET}
    main.cpp
)
target_link_libraries(${TARGET}
    registry-static
)

install(
    TARGETS ${TARGET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
)

# D-Bus service activation file
configure_file(
    com.canonical.StorageFramework.Registry.service.in
    com.canonical.StorageFramework.Registry.service)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/com.canonical.StorageFramework.Registry.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)

# systemd unit
configure_file(
    storage-framework-registry.service.in
    storage-framework-registry.service)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/storage-framework-registry.service
    DESTINATION "lib/systemd/user"
)
