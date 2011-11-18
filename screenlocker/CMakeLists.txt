add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1223)
add_subdirectory(greeter)

# for dbus_add_activation_service
include(PkgConfigGetVar)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)

set(screensaver_dbusXML dbus/org.freedesktop.ScreenSaver.xml)
set(kscreensaver_dbusXML dbus/org.kde.screensaver.xml)
set(ksmserver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)

set(ksld_SRCS
   ksldapp.cpp
   autologout.cpp
   interface.cpp
   lockwindow.cpp
   main.cpp)
qt4_add_dbus_adaptor(ksld_SRCS ${screensaver_dbusXML} interface.h ScreenLocker::Interface)
qt4_add_dbus_adaptor(ksld_SRCS ${kscreensaver_dbusXML} interface.h ScreenLocker::Interface kscreensaveradaptor KScreenSaverAdaptor)
kde4_add_kcfg_files(ksld_SRCS kcfg/kscreensaversettings.kcfgc)
QT4_ADD_DBUS_INTERFACE(ksld_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_kdeinit_executable(ksld ${ksld_SRCS})

target_link_libraries(kdeinit_ksld kworkspace kephal ${KDE4_KDEUI_LIBS} ${KDE4_KIDLETIME_LIBS} ${X11_LIBRARIES})

install(TARGETS kdeinit_ksld ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS ksld         ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES ksld.desktop  DESTINATION  ${AUTOSTART_INSTALL_DIR})
