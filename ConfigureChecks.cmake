include(UnixAuth)
set_package_properties(PAM PROPERTIES DESCRIPTION "PAM Libraries"
                       URL "https://www.kernel.org/pub/linux/libs/pam/"
                       TYPE OPTIONAL
                       PURPOSE "Required for screen unlocking and optionally used by the KDM log in manager"
                      )
include(CheckTypeSize)
include(FindPkgConfig)

macro_optional_find_package(XKB) # kxkb, kdm

if (PAM_FOUND)
    set(KDE4_COMMON_PAM_SERVICE "kde" CACHE STRING "The PAM service to use unless overridden for a particular app.")

    macro(define_pam_service APP)
        string(TOUPPER ${APP}_PAM_SERVICE var)
        set(cvar KDE4_${var})
        set(${cvar} "${KDE4_COMMON_PAM_SERVICE}" CACHE STRING "The PAM service for ${APP}.")
        mark_as_advanced(${cvar})
        set(${var} "\"${${cvar}}\"")
    endmacro(define_pam_service)

    macro(install_pam_service APP)
        string(TOUPPER KDE4_${APP}_PAM_SERVICE cvar)
        install(CODE "
            set(DESTDIR_VALUE \"\$ENV{DESTDIR}\")
            if (NOT DESTDIR_VALUE)
                exec_program(\"${KDEBASE_WORKSPACE_SOURCE_DIR}/mkpamserv\" ARGS ${${cvar}} RETURN_VALUE ret)
                if (NOT ret)
                    exec_program(\"${KDEBASE_WORKSPACE_SOURCE_DIR}/mkpamserv\" ARGS -P ${${cvar}}-np)
                endif (NOT ret)
            endif (NOT DESTDIR_VALUE)
                    ")
    endmacro(install_pam_service)

    define_pam_service(KDM)
    define_pam_service(kscreensaver)

else (PAM_FOUND)

    macro(install_pam_service APP)
    endmacro(install_pam_service)

endif (PAM_FOUND)

find_program(some_x_program NAMES iceauth xrdb xterm)
if (NOT some_x_program)
    set(some_x_program /usr/bin/xrdb)
    message("Warning: Could not determine X binary directory. Assuming /usr/bin.")
endif (NOT some_x_program)
get_filename_component(proto_xbindir "${some_x_program}" PATH)
get_filename_component(XBINDIR "${proto_xbindir}" ABSOLUTE)
get_filename_component(xrootdir "${XBINDIR}" PATH)
set(XLIBDIR "${xrootdir}/lib/X11")

check_function_exists(getpassphrase HAVE_GETPASSPHRASE)
check_function_exists(vsyslog HAVE_VSYSLOG)
check_function_exists(statvfs HAVE_STATVFS)

check_include_files(limits.h HAVE_LIMITS_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)     # ksmserver, ksplashml, sftp
check_include_files(stdint.h HAVE_STDINT_H)         # kcontrol/kfontinst
check_include_files("sys/stat.h;sys/vfs.h" HAVE_SYS_VFS_H) # statvfs for plasma/solid
check_include_files("sys/stat.h;sys/statvfs.h" HAVE_SYS_STATVFS_H) # statvfs for plasma/solid
check_include_files(sys/param.h HAVE_SYS_PARAM_H)
check_include_files("sys/param.h;sys/mount.h" HAVE_SYS_MOUNT_H)
check_include_files("sys/types.h;sys/statfs.h" HAVE_SYS_STATFS_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(malloc.h HAVE_MALLOC_H)
check_function_exists(statfs HAVE_STATFS)
macro_bool_to_01(FONTCONFIG_FOUND HAVE_FONTCONFIG) # kcontrol/{fonts,kfontinst}
macro_bool_to_01(FREETYPE_FOUND HAVE_FREETYPE) # kcontrol/fonts
macro_bool_to_01(OPENGL_FOUND HAVE_OPENGL) # kwin
macro_bool_to_01(X11_XShm_FOUND HAVE_XSHM) # kwin, ksplash
macro_bool_to_01(X11_XTest_FOUND HAVE_XTEST) # khotkeys, kxkb, kdm
macro_bool_to_01(X11_Xcomposite_FOUND HAVE_XCOMPOSITE) # kicker, kwin
macro_bool_to_01(X11_Xcursor_FOUND HAVE_XCURSOR) # many uses
macro_bool_to_01(X11_Xdamage_FOUND HAVE_XDAMAGE) # kwin
macro_bool_to_01(X11_Xfixes_FOUND HAVE_XFIXES) # klipper, kicker, kwin
if(WITH_XINERAMA)
    macro_bool_to_01(X11_Xinerama_FOUND HAVE_XINERAMA)
else(WITH_XINERAMA)
    set(HAVE_XINERAMA 0)
endif(WITH_XINERAMA)
macro_bool_to_01(X11_Xrandr_FOUND HAVE_XRANDR) # kwin
macro_bool_to_01(X11_Xrender_FOUND HAVE_XRENDER) # kcontrol/style, kicker
macro_bool_to_01(X11_xf86misc_FOUND HAVE_XF86MISC) # kdesktop and kcontrol/lock
macro_bool_to_01(X11_dpms_FOUND HAVE_DPMS) # kdesktop
macro_bool_to_01(X11_XSync_FOUND HAVE_XSYNC) # kwin

set(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
check_type_size("struct ucred" STRUCT_UCRED)       # kio_fonts

check_function_exists(getpeereid  HAVE_GETPEEREID) # kdesu
check_function_exists(setpriority  HAVE_SETPRIORITY) # kscreenlocker 

set(CMAKE_REQUIRED_INCLUDES ${X11_Xrandr_INCLUDE_PATH}/Xrandr.h)
set(CMAKE_REQUIRED_LIBRARIES ${X11_Xrandr_LIB})
check_function_exists(XRRGetScreenSizeRange XRANDR_1_2_FOUND)
macro_bool_to_01(XRANDR_1_2_FOUND HAS_RANDR_1_2)
check_function_exists(XRRGetScreenResourcesCurrent XRANDR_1_3_FOUND)
macro_bool_to_01(XRANDR_1_3_FOUND HAS_RANDR_1_3)
