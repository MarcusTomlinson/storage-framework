set(includeprefix unity/storage/provider)
file(GLOB provider_headers *.h)

install(FILES ${provider_headers}
  DESTINATION ${base_includedir}/${includeprefix})
