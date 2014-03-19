project(plasma-desktop)

plasma_install_package(desktoppackage org.kde.desktop plasma/shells shell)

# add_subdirectory(ktouchpadenabler)
# add_subdirectory(solid-actions-kcm)

# add_subdirectory(runners)
add_subdirectory(containments)
add_subdirectory(toolboxes)
add_subdirectory(applets)
add_subdirectory(kcms)
