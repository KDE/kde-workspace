# for dbus_add_activation_service
include(PkgConfigGetVar)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1223)
add_subdirectory(data)
add_subdirectory(greeter)

# for dbus_add_activation_service
include(PkgConfigGetVar)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)

set(screensaver_dbusXML dbus/org.freedesktop.ScreenSaver.xml)
set(kscreensaver_dbusXML dbus/org.kde.screensaver.xml)
set(ksmserver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
set(powerdevilpolicyagent_xml ${SOLID_DBUS_INTERFACES_DIR}/org.kde.Solid.PowerManagement.PolicyAgent.xml)

set(ksld_SRCS
   ksldapp.cpp
   autologout.cpp
   interface.cpp
   lockwindow.cpp)
qt5_add_dbus_adaptor(ksld_SRCS ${screensaver_dbusXML} interface.h ScreenLocker::Interface)
qt5_add_dbus_adaptor(ksld_SRCS ${kscreensaver_dbusXML} interface.h ScreenLocker::Interface kscreensaveradaptor KScreenSaverAdaptor)
kconfig_add_kcfg_files(ksld_SRCS kcfg/kscreensaversettings.kcfgc)
qt5_add_dbus_interface(ksld_SRCS ${ksmserver_xml} ksmserver_interface)
qt5_add_dbus_interface(ksld_SRCS ${powerdevilpolicyagent_xml} powerdevilpolicyagent)

add_library(screenlocker_static STATIC ${ksld_SRCS})

target_link_libraries(screenlocker_static kworkspace
   Qt5::X11Extras
   KF5::KIdleTime
   ${X11_LIBRARIES}
   ${X11_Xcursor_LIB}
   ${XCB_XCB_LIBRARY}
)

# Needed to compile on Arm target.
set_target_properties(screenlocker_static PROPERTIES COMPILE_FLAGS "-fPIC")

install(FILES kscreenlocker.notifyrc  DESTINATION ${DATA_INSTALL_DIR}/ksmserver/ RENAME ksmserver.notifyrc)
