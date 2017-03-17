include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GLIB_DEPS_INCLUDE_DIRS}
    ${ONLINEACCOUNTS_DEPS_INCLUDE_DIRS}
)

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

add_executable(${TARGET}
    internal/ListAccountsHandler.cpp
    internal/qdbus-last-error-msg.cpp
    internal/RegistryAdaptor.cpp
    main.cpp
    ${CMAKE_SOURCE_DIR}/include/unity/storage/registry/internal/ListAccountsHandler.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/registry/internal/RegistryAdaptor.h
    ${adaptor_files})

set_target_properties(${TARGET} PROPERTIES AUTOMOC TRUE)

target_link_libraries(${TARGET}
    storage-framework-common-internal
    Qt5::Core
    Qt5::DBus
    ${GLIB_DEPS_LDFLAGS}
    ${ONLINEACCOUNTS_DEPS_LDFLAGS}
)

install(
    TARGETS ${TARGET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
)

configure_file(com.canonical.StorageFramework.Registry.service.in com.canonical.StorageFramework.Registry.service)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/com.canonical.StorageFramework.Registry.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)
