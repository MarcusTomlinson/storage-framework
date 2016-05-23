set(src main.cpp)

add_executable(local-client main.cpp)
qt5_use_modules(local-client Core)
set_target_properties(local-client PROPERTIES AUTOMOC TRUE)
target_link_libraries(local-client storage-framework-qt-local-client Qt5::Core boost_filesystem)
