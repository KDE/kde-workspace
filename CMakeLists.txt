project(KDEBASE_WORKSPACE)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
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

set(_kdeworkspace_combined_build TRUE)

################# set KDE specific information #################

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

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

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core DBus Quick Widgets Concurrent X11Extras Test UiTools)
find_package(Qt5WebKitWidgets)

#include(ConfigureChecks.cmake)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

# Load the frameworks we need
find_package(KF5 CONFIG REQUIRED
    IdleTime ItemModels WidgetsAddons WindowSystem Codecs Archive CoreAddons Solid ThreadWeaver
    Config Auth JS JsEmbed Wallet DBusAddons Sonnet GlobalAccel
    I18n GuiAddons Service ConfigWidgets ItemViews Notifications IconThemes FrameworkIntegration Completion JobWidgets TextWidgets XmlGui Crash
    Bookmarks UnitConversion KIO KCMUtils Parts Kross NotifyConfig Su DocTools NewStuff KHtml Declarative Init Runner Activities Plasma PlasmaQuick KDE4Support)

include(ConfigureChecks.cmake)

# libs
# add_subdirectory(cmake)

add_subdirectory(libksysguard)

#
# system settings (kcontrol replacement)
ecm_optional_add_subdirectory( systemsettings )

add_subdirectory(plasma-workspace)
add_subdirectory(plasma-desktop)
add_subdirectory(oxygen)

#
# # core apps
if(NOT WIN32)
   ecm_optional_add_subdirectory( kwin )
   ecm_optional_add_subdirectory( powerdevil )
   if(Qt5WebKitWidgets_FOUND)
        if (NOT CMAKE_SYSTEM_NAME MATCHES Darwin)
            ecm_optional_add_subdirectory( ksysguard )
        endif ()
   endif()
endif()

ecm_optional_add_subdirectory(kmenuedit)
#    macro_optional_add_subdirectory(solid)

#
# data
ecm_optional_add_subdirectory(doc)

if (NOT WIN32)
   ecm_optional_add_subdirectory(kinfocenter)
endif ()

if(NOT WIN32)
   ecm_optional_add_subdirectory(khotkeys)
   ecm_optional_add_subdirectory(kwrited)
endif()

ecm_optional_add_subdirectory(kstyles)
#

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

set(KDE4WORKSPACE_TARGET_PREFIX PW::)

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


