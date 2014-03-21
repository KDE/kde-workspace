project(khotkeys)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")


set(KHOTKEYS_VERSION_MAJOR 4)
set(KHOTKEYS_VERSION_MINOR 90)
set(KHOTKEYS_VERSION_PATCH 1)
set(KHOTKEYS_VERSION_STRING "${KHOTKEYS_VERSION_MAJOR}.${KHOTKEYS_VERSION_MINOR}.${KHOTKEYS_VERSION_PATCH}" )

set(GENERIC_LIB_SOVERSION ${KHOTKEYS_VERSION_MAJOR})
set(GENERIC_LIB_VERSION "${KHOTKEYS_VERSION_STRING}")

set(KHOTKEYS_VERSION ${KHOTKEYS_VERSION_STRING})
set(KHOTKEYS_STRING_VERSION "${KHOTKEYS_VERSION_STRING}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeysprivate
    ${CMAKE_CURRENT_BINARY_DIR}/libkhotkeysprivate
    ${CMAKE_CURRENT_BINARY_DIR}
)

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    X11Extras
)

find_package(KF5 CONFIG REQUIRED COMPONENTS
    GlobalAccel
    I18n
    KIO
    XmlGui
    KDE4Support
    DBusAddons
    KCMUtils
)

find_package(X11 REQUIRED)

configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1217)
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

# Convenience library
add_subdirectory( libkhotkeysprivate )
# has to be before kcm_hotkeys
add_subdirectory( app )
# kcm module to configure hotkeys
add_subdirectory( kcm_hotkeys )
# kdedmodule and standalone demon for hotkeys
add_subdirectory( data )
# add_subdirectory( test )

