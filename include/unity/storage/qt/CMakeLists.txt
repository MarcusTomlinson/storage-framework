set(includeprefix unity/storage/qt)
file(GLOB public_hdrs *.h)
set(convenience_hdr ${CMAKE_CURRENT_BINARY_DIR}/client-api.h)

add_custom_command(
    OUTPUT ${convenience_hdr}
    COMMAND ${CMAKE_SOURCE_DIR}/tools/create_globalheader.py
        ${convenience_hdr} ${includeprefix} ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${public_hdrs})

add_custom_target(qt-client-all-headers ALL DEPENDS ${convenience_hdr})

install(
    FILES ${public_hdrs} ${convenience_hdr}
    DESTINATION ${client_base_includedir}/${includeprefix})
