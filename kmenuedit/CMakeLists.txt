project(kmenuedit)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMInstallIcons)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Xml
)

find_package(KF5 CONFIG REQUIRED COMPONENTS
    I18n
    XmlGui
    WindowSystem
    KCMUtils
    Solid
    Plasma
    DBusAddons
    IconThemes
    KIO
    Sonnet
    Init
    KDE4Support
)

find_package(KHotKeysDBusInterface QUIET)
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)
add_subdirectory( pixmaps ) 
add_subdirectory( doc )

set(kmenueditcommon_STAT_SRCS preferencesdlg.cpp klinespellchecking.cpp basictab.cpp treeview.cpp kmenuedit.cpp menufile.cpp menuinfo.cpp configurationmanager.cpp )
########### next target ###############

qt5_add_dbus_adaptor( kmenueditcommon_STAT_SRCS org.kde.kmenuedit.xml kmenuedit.h KMenuEdit)

if(NOT WIN32 AND ${KHotKeysDBusInterface_FOUND})
qt5_add_dbus_interface(
    kmenueditcommon_STAT_SRCS
    ${KHOTKEYS_DBUS_INTERFACE}
    khotkeys_interface)
set(kmenueditcommon_STAT_SRCS ${kmenueditcommon_STAT_SRCS} khotkeys.cpp)

add_definitions(-DWITH_HOTKEYS)
endif()

set(kmenuedit_KDEINIT_SRCS main.cpp ${kmenueditcommon_STAT_SRCS})

kf5_add_kdeinit_executable( kmenuedit ${kmenuedit_KDEINIT_SRCS})

target_link_libraries(kdeinit_kmenuedit
    Qt5::DBus
    Qt5::Xml
    KF5::DBusAddons
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::SonnetCore
    KF5::SonnetUi
    KF5::XmlGui
    KF5::KDE4Support
)

install(TARGETS kdeinit_kmenuedit  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kmenuedit kdeinit_kmenuedit )
install(TARGETS kmenuedit ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( PROGRAMS kmenuedit.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kmenueditui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmenuedit )

if(NOT WIN32)
add_dependencies(kdeinit_kmenuedit kded_khotkeys)
endif()

ecm_install_icons( ${ICON_INSTALL_DIR}  )

