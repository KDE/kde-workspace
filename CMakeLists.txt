project(KDEBASE_WORKSPACE)

cmake_minimum_required(VERSION 2.8.6 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

# Used e.g. in KDE4WorkspaceConfig.cmake, Alex
set(KDE4WORKSPACE_VERSION_MAJOR 4)
set(KDE4WORKSPACE_VERSION_MINOR 90)
set(KDE4WORKSPACE_VERSION_PATCH 1)
set(KDE4WORKSPACE_VERSION ${KDE4WORKSPACE_VERSION_MAJOR}.${KDE4WORKSPACE_VERSION_MINOR}.${KDE4WORKSPACE_VERSION_PATCH} )


set(GENERIC_LIB_SOVERSION ${KDE4WORKSPACE_VERSION_MAJOR})

set(GENERIC_LIB_VERSION "${KDE4WORKSPACE_VERSION_MAJOR}.${KDE4WORKSPACE_VERSION_MINOR}.${KDE4WORKSPACE_VERSION_PATCH}")


# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

################# set KDE specific information #################

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
include(ECMMarkNonGuiExecutable)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)

#TODO: this was coming from KDELibs4, we need a replacement
set(KDE_PLATFORM_PROFILE "Desktop")

find_package(Qt5Core REQUIRED NO_MODULE)
find_package(Qt5DBus REQUIRED NO_MODULE)
find_package(Qt5Quick REQUIRED NO_MODULE)
find_package(Qt5Widgets REQUIRED NO_MODULE)
find_package(Qt5WebKitWidgets REQUIRED NO_MODULE)
find_package(Qt5Concurrent REQUIRED NO_MODULE)
find_package(Qt5X11Extras REQUIRED NO_MODULE)
find_package(Qt5Test REQUIRED NO_MODULE)
find_package(Qt5UiTools REQUIRED NO_MODULE)

#include(ConfigureChecks.cmake)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

# Load the frameworks we need
find_package(KF5 CONFIG REQUIRED
    IdleTime ItemModels WidgetsAddons WindowSystem Codecs Archive CoreAddons Solid ThreadWeaver
    Config Auth JS Wallet DBusAddons Sonnet GlobalAccel
    I18n GuiAddons Service ConfigWidgets ItemViews Notifications IconThemes Style Completion JobWidgets TextWidgets XmlGui Crash
    Bookmarks UnitConversion KIO KCMUtils Parts Kross NotifyConfig PrintUtils Su DocTools NewStuff KHtml Declarative Init)

find_package(KF5Plasma REQUIRED NO_MODULE)
find_package(KF5PlasmaQuick REQUIRED NO_MODULE)

find_package(UDev) #PowerDevil upower/udev backend

# needed for set_package_properties macro
find_package(KF5KDE4Support REQUIRED NO_MODULE)

# find_package(Strigi)
# set_package_properties(Strigi PROPERTIES DESCRIPTION "Desktop indexing and search support"
#                        URL "http://strigi.sourceforge.net"
#                        TYPE REQUIRED
#                       )

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "Support for gzip compressed files and data streams"
                       URL "http://www.zlib.net"
                       TYPE REQUIRED
                      )

# find_package(KActivities 6.0.0 CONFIG)
# set_package_properties(KActivities PROPERTIES DESCRIPTION "Interface library for the activity manager"
#                        URL "https://projects.kde.org/kactivities"
#                        TYPE REQUIRED
#                       )

# TODO remove it in future
find_package(LibAttica NO_MODULE)
set_package_properties(LibAttica PROPERTIES DESCRIPTION "Support for Get Hot New Stuff related stuff"
                       URL "https://projects.kde.org/attica"
                       TYPE REQUIRED
                      )

find_package(DBusMenuQt5)
set_package_properties(DBusMenuQt5 PROPERTIES DESCRIPTION "Support for notification area menus via the DBusMenu protocol"
                        URL "https://launchpad.net/libdbusmenu-qt"
			TYPE OPTIONAL
                       )

find_package(QImageBlitz)
set_package_properties(QImageBlitz PROPERTIES DESCRIPTION "An image effects library"
                       URL "http://sourceforge.net/projects/qimageblitz"
                       TYPE OPTIONAL
                      )

# find_package(KF5Declarative QUIET CONFIG)
set_package_properties(KDeclarative PROPERTIES DESCRIPTION "KDE Declarative (QML) support from kdelibs"
                       URL "http://www.kde.org"
                       TYPE REQUIRED
                       PURPOSE "Required for building ksmserver"
                      )

find_package(OpenGL)
set_package_properties(OpenGL PROPERTIES DESCRIPTION "The OpenGL libraries"
                       URL "http://www.opengl.org"
                       TYPE OPTIONAL
                      )
find_package(OpenGLES)
set_package_properties(OpenGLES PROPERTIES DESCRIPTION "The OpenGLES libraries"
                       URL "http://www.khronos.org/opengles"
                       TYPE OPTIONAL
                      )

find_package(KActivities)
set_package_properties(KActivities PROPERTIES DESCRIPTION "The KActivities library"
                       URL "https://projects.kde.org/kactivities"
                       TYPE OPTIONAL
                      )

find_package(X11)
if (X11_FOUND)
   set (HAVE_X11 1)
else ()
   set (HAVE_X11 0)
endif ()
add_definitions(-DHAVE_X11=${HAVE_X11})

set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE OPTIONAL
                        PURPOSE "Required for building the X11 based workspace"
                    )

if(HAVE_X11)
  find_package(XCB REQUIRED COMPONENTS XCB XFIXES DAMAGE COMPOSITE SHAPE SYNC RENDER RANDR KEYSYMS IMAGE SHM XTEST)
  set_package_properties(XCB PROPERTIES DESCRIPTION "X protocol C-language Binding"
                         URL "http://xcb.freedesktop.org"
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
endif()

find_package(GLIB2 2.0)
set_package_properties(GLIB2 PROPERTIES DESCRIPTION "Low-level core library for data structure handling, portability wrappers, etc."
                       URL "http://www.gtk.org"
                       TYPE OPTIONAL
                       PURPOSE "Needed to build the kxkb keyboard map control program and provide XMMS support in the Now Playing Plasma data engine"
                      )

find_package(Fontconfig)
set_package_properties(Fontconfig PROPERTIES DESCRIPTION "Font access configuration library"
                       URL "http://www.freedesktop.org/wiki/Software/fontconfig"
                       TYPE OPTIONAL
                       PURPOSE "Needed to build font configuration and installation tools"
                      )

#### Python support ( plasma scriptengines ) ####
find_package(PythonLibrary)
set_package_properties(PythonLibrary PROPERTIES DESCRIPTION "Python scripting language"
                       URL "http://python.org"
                       TYPE OPTIONAL
                       PURPOSE "Needed to build plasma scriptengine for python."
                      )

find_package(Soprano 2.7.56)
set_package_properties(Soprano PROPERTIES DESCRIPTION "Semantic Desktop Storing"
                       URL "http://soprano.sourceforge.net"
                       TYPE OPTIONAL
                       PURPOSE "Required to build Nepomuk features."
                      )

find_package(NepomukCore QUIET CONFIG)
set_package_properties(NepomukCore PROPERTIES DESCRIPTION "Nepomuk Core Library"
                       URL "https://projects.kde.org/nepomuk-core"
                       TYPE OPTIONAL
                       PURPOSE "Required to build Nepomuk features."
                      )

find_package(Boost 1.34.0 MODULE)
set_package_properties(Boost PROPERTIES DESCRIPTION "Boost C++ Libraries"
                       URL "http://www.boost.org"
                       TYPE OPTIONAL
                       PURPOSE "Required to build certain Plasma DataEngines (Akonadi, RSS, Calendar)"
                      )

find_package(Akonadi QUIET CONFIG)
set_package_properties(Akonadi PROPERTIES DESCRIPTION "An extensible cross-desktop storage service for PIM data"
                       URL "http://pim.kde.org/akonadi"
                       TYPE OPTIONAL
                       PURPOSE "Required to build certain Plasma DataEngines (Akonadi, Calendar)"
                      )

find_package(KdepimLibs 4.5.60 QUIET CONFIG)
set_package_properties(KdepimLibs PROPERTIES DESCRIPTION "The KDEPIM libraries"
                       URL "http://pim.kde.org"
                       TYPE OPTIONAL
                       PURPOSE "Required to build certain Plasma DataEngines (Akonadi, Calendar)"
                      )

find_package(USB)
macro_bool_to_01(LIBUSB_FOUND HAVE_LIBUSB)
set_package_properties(USB PROPERTIES DESCRIPTION "User level access to USB devices"
                       URL "http://libusb.sourceforge.net"
                       TYPE OPTIONAL		
                       PURPOSE "Provides Logitech mouse support in KControl."
		       )

find_package(LibBSD)


include(ConfigureChecks.cmake)
if(NOT WIN32)
  configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
  configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h )
  if(NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
    set(EXPORT_XCURSOR_PATH "XCURSOR_PATH=${CMAKE_INSTALL_PREFIX}/share/icons:$XCURSOR_PATH\":~/.icons:/usr/share/icons:/usr/share/pixmaps:/usr/X11R6/lib/X11/icons\"; export XCURSOR_PATH")
  endif()
  configure_file(startkde.cmake ${CMAKE_CURRENT_BINARY_DIR}/startkde  @ONLY)
endif()
configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h )

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

################# Enable C++11 features for clang and gcc #################

if(UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++0x")
endif()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KACTIVITIES_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/libs)

add_definitions(-DDISABLE_NEPOMUK_LEGACY)

# libs
# add_subdirectory(cmake)
add_subdirectory(libs)
#
# system settings (kcontrol replacement)
ecm_optional_add_subdirectory( systemsettings )
#
# # core apps
if(NOT WIN32)
   ecm_optional_add_subdirectory( kcheckpass )
   ecm_optional_add_subdirectory( kwin )
   ecm_optional_add_subdirectory( ksmserver )
#
   find_package(JPEG REQUIRED)
   find_package(PNG REQUIRED)
#    macro_optional_add_subdirectory( ksplash )
#
    ecm_optional_add_subdirectory( powerdevil )
   if (NOT CMAKE_SYSTEM_NAME MATCHES Darwin)
      ecm_optional_add_subdirectory( ksysguard )
   endif ()
endif()
#
ecm_optional_add_subdirectory(kcontrol)
ecm_optional_add_subdirectory(klipper)
ecm_optional_add_subdirectory(kmenuedit)
#    macro_optional_add_subdirectory(krunner)
#    macro_optional_add_subdirectory(solid)
if(LIBBSD_FOUND)
    ecm_optional_add_subdirectory(kstartupconfig)
else()
    message(STATUS "LibBSD was not found, kstartupconfig will not be built")
endif()
ecm_optional_add_subdirectory(freespacenotifier)
#
# data
ecm_optional_add_subdirectory(doc)
#
if (NOT WIN32)
   ecm_optional_add_subdirectory(kinfocenter)
#       macro_optional_add_subdirectory(ktouchpadenabler)
endif ()

if(NOT WIN32)
   ecm_optional_add_subdirectory(kcminit)
   ecm_optional_add_subdirectory(khotkeys)
#       macro_optional_add_subdirectory(kwrited)
#       macro_optional_add_subdirectory(ksystraycmd)
   ecm_optional_add_subdirectory(appmenu)
endif()
#
if(X11_Xau_FOUND AND X11_Xdmcp_FOUND)
#       macro_optional_add_subdirectory( kdm )
else()
#       message(STATUS "Xau lib or Xdmcp lib was missing. kdm will not compile")
endif()
#
if(NOT WIN32)
    # data
    ecm_optional_add_subdirectory(cursors)
endif()
#
ecm_optional_add_subdirectory(plasma)
ecm_optional_add_subdirectory(statusnotifierwatcher)
ecm_optional_add_subdirectory(kstyles)
#
# ########### install files ###############
if(NOT WIN32)
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/startkde DESTINATION ${BIN_INSTALL_DIR})
endif()

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
   else ()
      set(${out} "\${KDE4WORKSPACE_INSTALL_DIR}/${in}")
   endif ()
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
#FIXME
#make_install_path_absolute(KDE4WORKSPACE_MIME_DIR    ${MIME_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SOUND_DIR   ${SOUND_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_TEMPLATES_DIR    ${TEMPLATES_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_WALLPAPER_DIR    ${WALLPAPER_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_KCONF_UPDATE_DIR    ${KCONF_UPDATE_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_AUTOSTART_DIR    ${AUTOSTART_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_XDG_APPS_DIR        ${XDG_APPS_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_XDG_DIRECTORY_DIR   ${XDG_DIRECTORY_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SYSCONF_DIR ${SYSCONF_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_MAN_DIR     ${MAN_INSTALL_DIR})
#FIXME
#make_install_path_absolute(KDE4WORKSPACE_INFO_DIR    ${INFO_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_DBUS_INTERFACES_DIR ${DBUS_INTERFACES_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_DBUS_SERVICES_DIR   ${DBUS_SERVICES_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SERVICES_DIR ${SERVICES_INSTALL_DIR})
make_install_path_absolute(KDE4WORKSPACE_SERVICETYPES_DIR ${SERVICETYPES_INSTALL_DIR})

set(KDE4WORKSPACE_TARGET_PREFIX KDE4Workspace__)
#
configure_file(KDE4WorkspaceConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfig.cmake" @ONLY)

# this file will be installed too and will be used by cmake when searching for the Config.cmake file to check the version of kdepimlibs, Alex
write_basic_config_version_file(${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfigVersion.cmake VERSION
                                     ${KDE4WORKSPACE_VERSION_MAJOR}.${KDE4WORKSPACE_VERSION_MINOR}.${KDE4WORKSPACE_VERSION_PATCH} COMPATIBILITY AnyNewerVersion)

set(_KDE4WorkspaceConfig_INSTALL_DIR ${LIB_INSTALL_DIR}/KDE4Workspace/cmake)

# places where find_package() looks for FooConfig.cmake files:
# CMake >= 2.6.0 looks in lib/Foo*/cmake/, CMake >= 2.6.3 also looks in
# lib/cmake/Foo*/, which packagers prefer. So they can set the KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR
# option to have kdepimlibs install its Config file there. Alex
if(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)
   set(_KDE4WorkspaceConfig_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KDE4Workspace)
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/KDE4WorkspaceConfigVersion.cmake
        DESTINATION ${_KDE4WorkspaceConfig_INSTALL_DIR})

install(EXPORT kdeworkspaceLibraryTargets
        NAMESPACE ${KDE4WORKSPACE_TARGET_PREFIX}
        DESTINATION ${_KDE4WorkspaceConfig_INSTALL_DIR}
        FILE KDE4WorkspaceLibraryTargets.cmake )


