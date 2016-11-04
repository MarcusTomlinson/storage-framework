set(includeprefix unity/storage)
file(GLOB common_headers *.h)

# Install common headers to both include prefixes
install(FILES ${common_headers}
  DESTINATION ${client_base_includedir}/${includeprefix})
install(FILES ${common_headers}
  DESTINATION ${provider_base_includedir}/${includeprefix})

# Deprecated client API v1 install
install(FILES ${common_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/storage-framework-client-1/${includeprefix})

add_subdirectory(provider)
add_subdirectory(qt)
