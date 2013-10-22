project(plasma-systemtray)

add_subdirectory(plugin)
add_subdirectory(tests)

plasma_install_package(org.kde.systemtray org.kde.systemtray)

plasma_install_package(tests/systrayplasmoidtest org.kde.systrayplasmoidtest)
