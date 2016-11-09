set(src
    AccountDetails.cpp
    dbusmarshal.cpp
    EnvVars.cpp
    InactivityTimer.cpp
    safe_strerror.cpp
    TraceMessageHandler.cpp
    ${CMAKE_SOURCE_DIR}/include/unity/storage/internal/InactivityTimer.h
)


add_library(storage-framework-common-internal STATIC ${src})

set_target_properties(storage-framework-common-internal PROPERTIES AUTOMOC TRUE)

target_link_libraries(storage-framework-common-internal
    Qt5::DBus
)
