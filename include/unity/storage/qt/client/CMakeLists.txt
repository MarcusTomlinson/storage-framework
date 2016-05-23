set(convenience_hdr client-api.h)
set(prefix unity/storage/qt/client)
file(GLOB public_hdrs *.h)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${convenience_hdr}
    COMMAND ${CMAKE_SOURCE_DIR}/tools/create_globalheader.py
        ${CMAKE_CURRENT_BINARY_DIR}/${convenience_hdr} ${prefix} ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${public_hdrs})

add_custom_target(qt-client-all-headers ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${convenience_hdr})

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${convenience_hdr}
    DESTINATION include/${prefix})
