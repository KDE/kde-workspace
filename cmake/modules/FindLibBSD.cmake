# - Try to find libbsd
# Once done this will define
#  LIBBSD_FOUND - System has libbsd
#  LIBBSD_INCLUDE_DIRS - The libbsd include directories
#  LIBBSD_LIBRARIES - The libraries needed to use libbsd

find_path(LIBBSD_INCLUDE_DIR bsd.h
          HINTS ${LIBBSD_INCLUDEDIR} ${LIBBSD_INCLUDE_DIRS} ${CMAKE_INSTALL_PREFIX}/include PATH_SUFFIXES bsd)

find_library(LIBBSD_LIBRARY NAMES libbsd.so
HINTS ${LIBBSD_LIBDIR} ${LIBBSD_LIBRARY_DIRS} ${CMAKE_INSTALL_PREFIX}/lib64)

set(LIBBSD_LIBRARIES ${LIBBSD_LIBRARY} )
set(LIBBSD_INCLUDE_DIRS ${LIBBSD_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set  LIBBSD_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libbsd  DEFAULT_MSG
                                  LIBBSD_LIBRARY LIBBSD_INCLUDE_DIR)

mark_as_advanced(LIBBSD_INCLUDE_DIR LIBBSD_LIBRARY )
