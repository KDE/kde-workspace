project(plasma-containmentactions-contextmenu)

set(contextmenu_SRCS
    menu.cpp
)

kservice_desktop_to_json(plasma-containmentactions-contextmenu.desktop)

set(krunner_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.kde.krunner.App.xml)
QT4_ADD_DBUS_INTERFACE(contextmenu_SRCS ${krunner_xml} krunner_interface)

set(screensaver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/screenlocker/dbus/org.freedesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE(contextmenu_SRCS ${screensaver_xml} screensaver_interface)


kde4_add_plugin(plasma_containmentactions_contextmenu ${contextmenu_SRCS})
target_link_libraries(plasma_containmentactions_contextmenu ${Plasma_LIBRARIES} ${KDE4_KIO_LIBS} kworkspace)

install(TARGETS plasma_containmentactions_contextmenu DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-contextmenu.desktop DESTINATION ${SERVICES_INSTALL_DIR})
