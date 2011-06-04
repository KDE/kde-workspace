# - Try to find the Wayland directory library
# Once done this will define
#
#  WAYLAND_FOUND - system has Wayland libraries
#  WAYLAND_SERVER_FOUND - system has Wayland server libraries
#  WAYLAND_CLIENT_FOUND - system has Wayland client libraries
#  WAYLAND_SERVER_INCLUDE_DIR - Wayland Server include directory
#  WAYLAND_SERVER_LIBRARIES - The libraries needed for Wayland Server
#  WAYLAND_CLIENT_INCLUDE_DIR - Wayland Client include directory
#  WAYLAND_CLIENT_LIBRARIES - The libraries needed for Wayland Clients
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(_WAYLAND_SERVER_PC QUIET wayland-server)
    pkg_check_modules(_WAYLAND_CLIENT_PC QUIET wayland-client)
endif (PKG_CONFIG_FOUND)

# Wayland server
FIND_PATH(WAYLAND_SERVER_INCLUDE_DIR wayland-server.h
    ${_WAYLAND_SERVER_PC_INCLUDE_DIRS}
)

FIND_LIBRARY(WAYLAND_SERVER_LIBRARIES NAMES wayland-server
    PATHS
    ${_WAYLAND_SERVER_PC_LIBDIR}
)

if (WAYLAND_SERVER_INCLUDE_DIR AND WAYLAND_SERVER_LIBRARIES)
    set(WAYLAND_SERVER_FOUND TRUE)
endif (WAYLAND_SERVER_INCLUDE_DIR AND WAYLAND_SERVER_LIBRARIES)

# Wayland Client
FIND_PATH(WAYLAND_CLIENT_INCLUDE_DIR wayland-client.h
    ${_WAYLAND_CLIENT_PC_INCLUDE_DIRS}
)

FIND_LIBRARY(WAYLAND_CLIENT_LIBRARIES NAMES wayland-client
    PATHS
    ${_WAYLAND_CLIENT_PC_LIBDIR}
)

if (WAYLAND_CLIENT_INCLUDE_DIR AND WAYLAND_CLIENT_LIBRARIES)
    set(WAYLAND_CLIENT_FOUND TRUE)
endif (WAYLAND_CLIENT_INCLUDE_DIR AND WAYLAND_CLIENT_LIBRARIES)

if (WAYLAND_SERVER_FOUND AND WAYLAND_CLIENT_FOUND)
    set(WAYLAND_FOUND TRUE)
endif (WAYLAND_SERVER_FOUND AND WAYLAND_CLIENT_FOUND)

macro_log_feature(WAYLAND_FOUND "Wayland" "Wayland Libraries" "http://wayland.freedesktop.org" FALSE)

MARK_AS_ADVANCED(WAYLAND_SERVER_INCLUDE_DIR WAYLAND_SERVER_LIBRARIES WAYLAND_CLIENT_INCLUDE_DIR WAYLAND_CLIENT_LIBRARIES)
