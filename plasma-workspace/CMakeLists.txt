project(plasma-workspace)

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "Support for gzip compressed files and data streams"
                       URL "http://www.zlib.net"
                       TYPE REQUIRED
                      )

find_package(dbusmenu-qt5 CONFIG)
set_package_properties(dbusmenu-qt5 PROPERTIES DESCRIPTION "Support for notification area menus via the DBusMenu protocol"
                       URL "https://launchpad.net/libdbusmenu-qt"
                       TYPE OPTIONAL
                      )

find_package(QImageBlitz)
set_package_properties(QImageBlitz PROPERTIES DESCRIPTION "An image effects library"
                       URL "http://sourceforge.net/projects/qimageblitz"
                       TYPE OPTIONAL
                      )

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE OPTIONAL
                        PURPOSE "Required for building the X11 based workspace")

find_package(KdepimLibs 4.70.10 QUIET CONFIG)
set_package_properties(KdepimLibs PROPERTIES DESCRIPTION "The KDEPIM libraries"
                       URL "http://pim.kde.org"
                       TYPE OPTIONAL
                       PURPOSE "Required to build certain Plasma DataEngines (Akonadi, Calendar)"
                      )

find_package(Akonadi QUIET CONFIG)
set_package_properties(Akonadi PROPERTIES DESCRIPTION "An extensible cross-desktop storage service for PIM data"
                       URL "http://pim.kde.org/akonadi"
                       TYPE OPTIONAL
                       PURPOSE "Required to build certain Plasma DataEngines (Akonadi, Calendar)"
                      )

find_package(Boost 1.34.0 MODULE)
set_package_properties(Boost PROPERTIES DESCRIPTION "Boost C++ Libraries"
                       URL "http://www.boost.org"
                       TYPE OPTIONAL
                       PURPOSE "Required to build certain Plasma DataEngines (Akonadi, RSS, Calendar)"
                      )

if(X11_FOUND)
  find_package(XCB REQUIRED COMPONENTS XCB XFIXES DAMAGE COMPOSITE SHAPE SYNC RENDER RANDR KEYSYMS IMAGE SHM XTEST)
  set_package_properties(XCB PROPERTIES TYPE REQUIRED)
  find_package(XCB COMPONENTS ICCCM)
  add_feature_info("XCB-ICCCM" XCB_ICCCM_FOUND "Required for building test applications for KWin")
endif()

set(PLASMAWORKSPACE_VERSION_MAJOR 4)
set(PLASMAWORKSPACE_VERSION_MINOR 90)
set(PLASMAWORKSPACE_VERSION_PATCH 1)
set(PLASMAWORKSPACE_VERSION_STRING "${PLASMAWORKSPACE_VERSION_MAJOR}.${PLASMAWORKSPACE_VERSION_MINOR}.${PLASMAWORKSPACE_VERSION_PATCH}" )

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h)
configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)

plasma_install_package(lookandfeel org.kde.lookandfeel plasma/look-and-feel lookandfeel)

add_subdirectory(doc)
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
add_subdirectory(themes)

add_subdirectory(containmentactions)
add_subdirectory(runners)
add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(wallpapers)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
