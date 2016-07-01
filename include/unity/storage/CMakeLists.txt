set(includeprefix unity/storage)
file(GLOB common_headers *.h)

install(FILES ${common_headers}
  DESTINATION ${base_includedir}/${includeprefix})

add_subdirectory(provider)
add_subdirectory(qt)
