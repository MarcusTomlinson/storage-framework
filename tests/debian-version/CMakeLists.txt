add_test(
  NAME debian-version
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/check-debian-version.sh "${PROJECT_VERSION}" "${CMAKE_SOURCE_DIR}"
)
