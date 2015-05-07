project(KDEBASE_WORKSPACE)
# set_package_properties appeared in cmake 2.8.6
# TODO: Remove when kdelibs >= 4.10 will be required
cmake_minimum_required(VERSION 2.8.6 FATAL_ERROR)

# Used e.g. in KDE4WorkspaceConfig.cmake, Alex
set(KDE4WORKSPACE_VERSION_MAJOR 4)
set(KDE4WORKSPACE_VERSION_MINOR 11)
set(KDE4WORKSPACE_VERSION_PATCH 19)
set(KDE4WORKSPACE_VERSION ${KDE4WORKSPACE_VERSION_MAJOR}.${KDE4WORKSPACE_VERSION_MINOR}.${KDE4WORKSPACE_VERSION_PATCH} )

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

#search packages used by KDE
set(QT_MIN_VERSION "4.8.0")

find_package(KDE4 4.9.4 REQUIRED)
include(KDE4Defaults)

option(WITH_XINERAMA "Xinerama support for multi-headed X displays" ON)

find_package(Strigi)

set_package_properties(Strigi PROPERTIES DESCRIPTION "Desktop indexing and search support"
                       URL "http://strigi.sourceforge.net"
                       TYPE REQUIRED
                      )

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "Support for gzip compressed files and data streams"
                       URL "http://www.zlib.net"
                       TYPE REQUIRED
                      )

find_package(KActivities 6.0.0 CONFIG)
set_package_properties(KActivities PROPERTIES DESCRIPTION "Interface library for the activity manager"
                       URL "https://projects.kde.org/kactivities"
                       TYPE REQUIRED
                      )

find_package(DBusMenuQt 0.6.0)
set_package_properties(DBusMenuQt PROPERTIES DESCRIPTION "Support for notification area menus via the DBusMenu protocol"
                       URL "https://launchpad.net/libdbusmenu-qt"
                       TYPE REQUIRED
                      )

find_package(QImageBlitz)
set_package_properties(QImageBlitz PROPERTIES DESCRIPTION "An image effects library"
                       URL "http://sourceforge.net/projects/qimageblitz"
                       TYPE REQUIRED
                      )

find_package(KDeclarative QUIET CONFIG)
set_package_properties(KDeclarative PROPERTIES DESCRIPTION "KDE Declarative (QML) support from kdelibs"
                       URL "http://www.kde.org"
                       TYPE REQUIRED
                       PURPOSE "Required for building ksmserver"
                      )

macro_optional_find_package(OpenGL)
set_package_properties(OpenGL PROPERTIES DESCRIPTION "The OpenGL libraries"
                       URL "http://www.opengl.org"
                       TYPE OPTIONAL
                      )
macro_optional_find_package(OpenGLES)
set_package_properties(OpenGLES PROPERTIES DESCRIPTION "The OpenGLES libraries"
                       URL "http://www.khronos.org/opengles"
                       TYPE OPTIONAL
                      )
macro_optional_find_package(UDev)
set_package_properties(UDev PROPERTIES DESCRIPTION "The UDev Libraries"
                       TYPE OPTIONAL
                       PURPOSE "Allows support for UPower backend in PowerDevil - STRONGLY RECOMMENDED"
                      )

macro_optional_find_package(Wayland)
set_package_properties(Wayland PROPERTIES DESCRIPTION "The Wayland Client and Server libraries"
                       URL "http://wayland.freedesktop.org"
                       TYPE OPTIONAL
                       PURPOSE "Required for building KWin with Wayland support"
                      )
if(Q_WS_X11)
  find_package(XCB REQUIRED)
  set_package_properties(XCB PROPERTIES DESCRIPTION "X protocol C-language Binding"
                         URL "http://xcb.freedesktop.org"
                         TYPE REQUIRED
                        )

  find_package(X11_XCB)
  set_package_properties(X11_XCB PROPERTIES DESCRIPTION "XCB X11 protocol client library"
                         TYPE REQUIRED
                        )

  find_package(X11 REQUIRED)
  set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                         URL "http://www.x.org"
                         TYPE REQUIRED
                        )

  add_feature_info("Automated testing of X clients" X11_XTest_FOUND
                   "The X11 Testing Resource extension library is useful for automated testing of X clients")
  add_feature_info("libXau" X11_Xau_FOUND "The X11 Authorization Protocol library may be used by KDM")
  add_feature_info("LibXdmcp" X11_Xdmcp_FOUND "The X Display Manager Control Protocol library may be used by KDM")
  if(NOT X11_Xkbfile_FOUND)
    message(FATAL_ERROR "The X11 keyboard layout library was not found. Required for building keyboard modules.")
  endif()
  if(NOT X11_Xcomposite_FOUND)
    message(FATAL_ERROR "The X11 composite library was not found. Required for building the plasma tray and notifications widgets.")
  endif()
  if(NOT X11_Xdamage_FOUND)
    message(FATAL_ERROR "The X11 damaged region extension library was not found. Required for compositing support in KWin.")
  endif()
  if(NOT X11_Xrender_FOUND)
    message(FATAL_ERROR " The X Rendering Extension client library was not found. Required for XRender Compositing backend in KWin.")
  endif()
  if(NOT X11_Xfixes_FOUND)
    message(FATAL_ERROR "The X11 miscellaneous 'fixes' extension library was not found. Required for XRender Compositing backend in KWin.")
  endif()
  if(NOT X11_Xrandr_FOUND)
    message(FATAL_ERROR "The X11 RandR extension library was not found. Required for Multi Screen Support.")
  endif()
  if(NOT OPENGL_FOUND AND NOT OPENGLES_FOUND)
    message(FATAL_ERROR "Either OpenGL or OpenGL ES 2.0 are required for Compositing support in KWin.")
  endif()
  if(NOT X11_Xcursor_FOUND)
    message(FATAL_ERROR "The X11 cursor management library was not found. Required for desktop effects support in KWin.")
  endif()
endif(Q_WS_X11)

macro_optional_find_package(GLIB2 2.0)
set_package_properties(GLIB2 PROPERTIES DESCRIPTION "Low-level core library for data structure handling, portability wrappers, etc."
                       URL "http://www.gtk.org"
                       TYPE OPTIONAL
                       PURPOSE "Needed to build the kxkb keyboard map control program and provide XMMS support in the Now Playing Plasma data engine"
                      )

macro_optional_find_package(Fontconfig)
set_package_properties(Fontconfig PROPERTIES DESCRIPTION "Font access configuration library"
                       URL "http://www.freedesktop.org/wiki/Software/fontconfig"
                       TYPE OPTIONAL
                       PURPOSE "Needed to build font configuration and installation tools"
                      )

#### Python support ( plasma scriptengines ) ####
macro_optional_find_package(PythonLibrary)
set_package_properties(PythonLibrary PROPERTIES DESCRIPTION "Python scripting language"
                       URL "http://python.org"
                       TYPE OPTIONAL
                       PURPOSE "Needed to build plasma scriptengine for python."
                      )

macro_optional_find_package(Soprano 2.7.56)
set_package_properties(Soprano PROPERTIES DESCRIPTION "Semantic Desktop Storing"
                       URL "http://soprano.sourceforge.net"
                       TYPE OPTIONAL
                       PURPOSE "Required to build Nepomuk features."
                      )

macro_optional_find_package(NepomukCore QUIET CONFIG)
set_package_properties(NepomukCore PROPERTIES DESCRIPTION "Nepomuk Core Library"
                       URL "https://projects.kde.org/nepomuk-core"
                       TYPE OPTIONAL
                       PURPOSE "Required to build Nepomuk features."
                      )

macro_optional_find_package(Boost 1.34.0 MODULE)
set_package_properties(Boost PROPERTIES DESCRIPTION "Boost C++ Libraries"
                       URL "http://www.boost.org"
                       TYPE OPTIONAL
                       PURPOSE "Required to build certain Plasma DataEngines (Akonadi, RSS, Calendar)"
                      )

macro_optional_find_package(Akonadi QUIET CONFIG)
set_package_properties(Akonadi PROPERTIES DESCRIPTION "An extensible cross-desktop storage service for PIM data"
                       URL "http://pim.kde.org/akonadi"
                       TYPE OPTIONAL
                       PURPOSE "Required to build certain Plasma DataEngines (Akonadi, Calendar)"
                      )

macro_optional_find_package(KdepimLibs 4.10.68 QUIET CONFIG)
set_package_properties(KdepimLibs PROPERTIES DESCRIPTION "The KDEPIM libraries"
                       URL "http://pim.kde.org"
                       TYPE OPTIONAL
                       PURPOSE "Required to build certain Plasma DataEngines (Akonadi, Calendar)"
                      )

macro_optional_find_package(QJSON)
set_package_properties(QJSON PROPERTIES DESCRIPTION "Library to manage JSON objects with Qt"
                       URL "http://qjson.sourceforge.net/"
                       TYPE OPTIONAL
                       PURPOSE "Required to build Chrome/Chromium support for Plasma Bookmarks Runners"
                      )


include(ConfigureChecks.cmake)
if(NOT WIN32)
  configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
  configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h )
  if(NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
    set(EXPORT_XCURSOR_PATH "XCURSOR_PATH=${CMAKE_INSTALL_PREFIX}/share/icons:$XCURSOR_PATH\":~/.icons:/usr/share/icons:/usr/share/pixmaps:/usr/X11R6/lib/X11/icons\"; export XCURSOR_PATH")
  endif(NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
  configure_file(startkde.cmake ${CMAKE_CURRENT_BINARY_DIR}/startkde  @ONLY)
endif(NOT WIN32)
configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h )

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KACTIVITIES_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/libs)

add_definitions(-DDISABLE_NEPOMUK_LEGACY)

# libs
add_subdirectory(cmake)
add_subdirectory(libs)

# system settings (kcontrol replacement)
if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   macro_optional_add_subdirectory( systemsettings )
endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")

# core apps
if(NOT WIN32)
   macro_optional_add_subdirectory( kcheckpass )
   macro_optional_add_subdirectory( kwin )
   macro_optional_add_subdirectory( ksmserver )

   find_package(JPEG REQUIRED)
   find_package(PNG REQUIRED)
   macro_optional_add_subdirectory( ksplash )

   macro_optional_add_subdirectory( powerdevil )
   macro_optional_add_subdirectory( qguiplatformplugin_kde )
   if (NOT CMAKE_SYSTEM_NAME MATCHES Darwin)
      macro_optional_add_subdirectory( ksysguard )
   endif (NOT CMAKE_SYSTEM_NAME MATCHES Darwin)
endif(NOT WIN32)

if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   macro_optional_add_subdirectory(kcontrol)
   macro_optional_add_subdirectory(klipper)
   macro_optional_add_subdirectory(kmenuedit)
   macro_optional_add_subdirectory(krunner)
   macro_optional_add_subdirectory(solid-actions-kcm)
   macro_optional_add_subdirectory(kstartupconfig)
   macro_optional_add_subdirectory(freespacenotifier)
   macro_optional_add_subdirectory(kscreensaver)
   macro_optional_add_subdirectory(kinfocenter)

   # data
   macro_optional_add_subdirectory(doc)

   if (Q_WS_X11)
      macro_optional_add_subdirectory(ktouchpadenabler)
   endif (Q_WS_X11)

   if(NOT WIN32)
      macro_optional_add_subdirectory(kcminit)
      macro_optional_add_subdirectory(khotkeys)
      macro_optional_add_subdirectory(kwrited)
      macro_optional_add_subdirectory(ksystraycmd)
      macro_optional_add_subdirectory(appmenu)
   endif(NOT WIN32)

   if(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
      macro_optional_add_subdirectory( kdm )
   else(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
      message(STATUS "Xau lib or Xdmcp lib was missing. kdm will not compile")
   endif(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")

if(NOT WIN32)
    # data
    macro_optional_add_subdirectory(cursors)
endif(NOT WIN32)

macro_optional_add_subdirectory(plasma)
macro_optional_add_subdirectory(statusnotifierwatcher)
macro_optional_add_subdirectory(kstyles)

########### install files ###############
if(NOT WIN32)
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/startkde DESTINATION ${BIN_INSTALL_DIR})
endif(NOT WIN32)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

# make the libraries installed from kdebase/workspace available to other projects
# by creating and installing a KDE4WorkspaceConfig.cmake file, which will
# be searched and found by kdelibs/cmake/modules/FindKDE4Workspace.cmake. Alex

# now create the KDE4WorkspaceConfig.cmake file, which will be loaded by 
# kdelibs/cmake/modules/FindKDE4Workspace.cmake and which has to contain all information
# about the libraries installed from kdebase/workspace/libs/ anybody would like to have. Alex

# we need the absolute directories where stuff will be installed too
# but since the variables which contain the destinations can be relative
# or absolute paths, we need this macro to make them all absoulte, Alex
macro(MAKE_INSTALL_PATH_ABSOLUTE out in)
   if (IS_ABSOLUTE "${in}")    # IS_ABSOLUTE is new since cmake 2.4.8
      set(${out} "${in}")
   else (IS_ABSOLUTE "${in}")
      set(${out} "\${KDE4WORKSPACE_INSTALL_DIR}/${in}")
   endif (IS_ABSOLUTE "${in}")
endmacro(MAKE_INSTALL_PATH_ABSOLUTE out in)

make_install_path_absolute(KDE4WORKSPACE_LIB_DIR     ${LIB_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_LIBEXEC_DIR ${LIBEXEC_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_INCLUDE_DIR ${INCLUDE_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_BIN_DIR     ${BIN_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SBIN_DIR    ${SBIN_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_DATA_DIR    ${DATA_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_HTML_DIR    ${HTML_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_CONFIG_DIR  ${CONFIG_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_ICON_DIR    ${ICON_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_KCFG_DIR    ${KCFG_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_LOCALE_DIR  ${LOCALE_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_MIME_DIR    ${MIME_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SOUND_DIR   ${SOUND_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_TEMPLATES_DIR    ${TEMPLATES_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_WALLPAPER_DIR    ${WALLPAPER_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_KCONF_UPDATE_DIR    ${KCONF_UPDATE_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_AUTOSTART_DIR    ${AUTOSTART_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_XDG_APPS_DIR        ${XDG_APPS_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_XDG_DIRECTORY_DIR   ${XDG_DIRECTORY_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SYSCONF_DIR ${SYSCONF_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_MAN_DIR     ${MAN_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_INFO_DIR    ${INFO_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_DBUS_INTERFACES_DIR ${DBUS_INTERFACES_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_DBUS_SERVICES_DIR   ${DBUS_SERVICES_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SERVICES_DIR ${SERVICES_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SERVICETYPES_DIR ${SERVICETYPES_INSTALL_DIR})

set(KDE4WORKSPACE_TARGET_PREFIX KDE4Workspace__)

configure_file(KDE4WorkspaceConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfig.cmake" @ONLY)

# this file will be installed too and will be used by cmake when searching for the Config.cmake file to check the version of kdepimlibs, Alex
macro_write_basic_cmake_version_file(${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfigVersion.cmake 
                                     ${KDE4WORKSPACE_VERSION_MAJOR} ${KDE4WORKSPACE_VERSION_MINOR} ${KDE4WORKSPACE_VERSION_PATCH})

set(_KDE4WorkspaceConfig_INSTALL_DIR ${LIB_INSTALL_DIR}/KDE4Workspace/cmake)
# places where find_package() looks for FooConfig.cmake files:
# CMake >= 2.6.0 looks in lib/Foo*/cmake/, CMake >= 2.6.3 also looks in
# lib/cmake/Foo*/, which packagers prefer. So they can set the KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR
# option to have kdepimlibs install its Config file there. Alex
if(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)
   set(_KDE4WorkspaceConfig_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KDE4Workspace)
endif(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfigVersion.cmake
        DESTINATION ${_KDE4WorkspaceConfig_INSTALL_DIR})

install(EXPORT kdeworkspaceLibraryTargets 
        NAMESPACE ${KDE4WORKSPACE_TARGET_PREFIX}
        DESTINATION ${_KDE4WorkspaceConfig_INSTALL_DIR}
        FILE KDE4WorkspaceLibraryTargets.cmake )


