pkg_check_modules(QTDBUSTEST_DEPS libqtdbustest-1 REQUIRED)

add_library(testutils STATIC
  DBusEnvironment.cpp
)
target_compile_options(testutils PRIVATE ${QTDBUSTEST_DEPS_CFLAGS})
target_link_libraries(testutils
  PUBLIC Qt5::DBus Qt5::Core
  PRIVATE ${QTDBUSTEST_DEPS_LIBRARIES})
