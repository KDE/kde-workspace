# project(khotkeys)
#
# cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
# set(QT_MIN_VERSION "5.2.0")
#
# include_directories(
#     ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeysprivate
#     ${CMAKE_CURRENT_BINARY_DIR}/libkhotkeysprivate
# )
#
# find_package(ECM 0.0.11 REQUIRED NO_MODULE)
# set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
#
# find_package(X11)
# set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
#                         URL "http://www.x.org"
#                         TYPE REQUIRED)
#
# find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
#     X11Extras
# )
#
# find_package(KF5 CONFIG REQUIRED COMPONENTS
#     GlobalAccel
#     I18n
#     KIO
#     KIOWidgets
#     XmlGui
#     KDE4Support
# )

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

