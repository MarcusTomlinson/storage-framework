include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Qt5Network_INCLUDE_DIRS}
)

add_definitions(-DBOOST_THREAD_VERSION=4)

add_library(local-provider-lib STATIC
    LocalDownloadJob.cpp
    LocalProvider.cpp
    LocalUploadJob.cpp
    utils.cpp
)

target_include_directories(local-provider-lib PRIVATE
    ${GLIB_DEPS_INCLUDE_DIRS}
    ${GIO_DEPS_INCLUDE_DIRS}
)

set_target_properties(local-provider-lib PROPERTIES
    AUTOMOC TRUE
    POSITION_INDEPENDENT_CODE TRUE
)

set(TARGET storage-provider-local)

add_executable(${TARGET}
    main.cpp
)

target_link_libraries(${TARGET}
    local-provider-lib
    storage-framework-provider
    Qt5::Network
    ${GLIB_DEPS_LIBRARIES}
    ${GIO_DEPS_LIBRARIES}
)

install(
    TARGETS ${TARGET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
)
