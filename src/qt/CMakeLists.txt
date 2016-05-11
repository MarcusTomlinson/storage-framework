find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)

add_definitions(-DQT_NO_KEYWORDS)

add_subdirectory(client)
add_subdirectory(local_client)
