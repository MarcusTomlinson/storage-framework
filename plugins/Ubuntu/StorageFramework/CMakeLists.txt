add_library(storage-framework-qml MODULE
  plugin.cpp
)

set_target_properties(storage-framework-qml PROPERTIES
  AUTOMOC TRUE
  NO_SONAME TRUE
  LINK_FLAGS "-Wl,--no-undefined"
)

target_link_libraries(storage-framework-qml
  storage-framework-qt-client
  Qt5::DBus
  Qt5::Qml
)

configure_file(qmldir qmldir)
