add_library(qt-client-lib-local OBJECT
    AccountImpl.cpp
    DownloaderImpl.cpp
    FileImpl.cpp
    FolderImpl.cpp
    ItemImpl.cpp
    RootImpl.cpp
    Runtime_create.cpp
    RuntimeImpl.cpp
    UploaderImpl.cpp
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/local_client/DownloaderImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/local_client/UploaderImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/local_client/RuntimeImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/RuntimeBase.h
)
target_include_directories(qt-client-lib-local PRIVATE
  ${GLIB_DEPS_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Concurrent_INCLUDE_DIRS})
set_target_properties(qt-client-lib-local PROPERTIES
  AUTOMOC TRUE
  POSITION_INDEPENDENT_CODE TRUE
)
