add_definitions(-DBOOST_THREAD_VERSION=4)

add_library(local-provider-lib STATIC
    LocalDownloadJob.cpp
    LocalProvider.cpp
    LocalUploadJob.cpp
    utils.cpp
)

target_include_directories(local-provider-lib PRIVATE
    ${Qt5Network_INCLUDE_DIRS}
    ${GLIB_DEPS_INCLUDE_DIRS}
    ${GIO_DEPS_INCLUDE_DIRS}
)

set_target_properties(local-provider-lib PROPERTIES
    AUTOMOC TRUE
    POSITION_INDEPENDENT_CODE TRUE
)

add_executable(storage-provider-local
    main.cpp
)

target_link_libraries(storage-provider-local
    local-provider-lib
    storage-framework-provider
    Qt5::Network
    ${GLIB_DEPS_LIBRARIES}
    ${GIO_DEPS_LIBRARIES}
)

install(
    TARGETS storage-provider-local
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
)
