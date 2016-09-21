set(includeprefix unity/storage/qt/client)
file(GLOB public_hdrs *.h)
set(convenience_hdr ${CMAKE_CURRENT_BINARY_DIR}/client-api.h)

add_custom_command(
    OUTPUT ${convenience_hdr}
    COMMAND ${CMAKE_SOURCE_DIR}/tools/create_globalheader.py
        ${convenience_hdr} ${includeprefix} ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${public_hdrs})

add_custom_target(qt-client-all-headers-v1 ALL DEPENDS ${convenience_hdr})

install(FILES ${public_hdrs} ${convenience_hdr}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/storage-framework-client-1/${includeprefix})
