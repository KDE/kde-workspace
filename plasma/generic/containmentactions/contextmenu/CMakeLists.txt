project(plasma-containmentactions-contextmenu)

set(contextmenu_SRCS
    menu.cpp
)

kservice_desktop_to_json(plasma-containmentactions-contextmenu.desktop)

set(krunner_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.kde.krunner.App.xml)
qt5_add_dbus_interface(contextmenu_SRCS ${krunner_xml} krunner_interface)

set(screensaver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/screenlocker/dbus/org.freedesktop.ScreenSaver.xml)
qt5_add_dbus_interface(contextmenu_SRCS ${screensaver_xml} screensaver_interface)


add_library(plasma_containmentactions_contextmenu MODULE ${contextmenu_SRCS})
target_link_libraries(plasma_containmentactions_contextmenu
                      Qt5::DBus
                      KF5::I18n
                      KF5::Plasma
                      KF5::XmlGui
		      KF5::KIOCore
                      kworkspace)

install(TARGETS plasma_containmentactions_contextmenu DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-contextmenu.desktop DESTINATION ${SERVICES_INSTALL_DIR})
