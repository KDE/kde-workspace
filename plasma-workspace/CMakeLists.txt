project(plasma-workspace)

plasma_install_package(lookandfeel org.kde.lookandfeel plasma/look-and-feel lookandfeel)

add_subdirectory(libkworkspace)
add_subdirectory(libtaskmanager)

add_subdirectory(appmenu)
add_subdirectory(freespacenotifier)
add_subdirectory(klipper)
add_subdirectory(krunner)
add_subdirectory(ksmserver)
add_subdirectory(ksplash)
add_subdirectory(statusnotifierwatcher)
add_subdirectory(startkde)

add_subdirectory(containmentactions)
add_subdirectory(runners)
add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(wallpapers)
