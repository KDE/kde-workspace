
#######################################################################################
# Package
install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.kickoff)


#######################################################################################
# Kickoff Library

configure_file(core/config-kickoff-applets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kickoff-applets.h)

set(libkickoff_SRCS
    core/applicationmodel.cpp
    core/favoritesmodel.cpp
    core/itemhandlers.cpp
    core/kickoffabstractmodel.cpp
    core/kickoffmodel.cpp
    core/kickoffproxymodel.cpp
    core/krunnermodel.cpp
    core/leavemodel.cpp
    core/models.cpp
    core/recentapplications.cpp
    core/recentlyusedmodel.cpp
    core/systemmodel.cpp
    core/urlitemlauncher.cpp
)

qt4_automoc(${libkickoff_SRCS})

qt4_add_dbus_adaptor(libkickoff_SRCS core/org.kde.kickoff.xml core/applicationmodel.h Kickoff::ApplicationModel)
qt4_add_dbus_adaptor(libkickoff_SRCS core/org.kde.kickoff.recent.xml core/recentlyusedmodel.h Kickoff::RecentlyUsedModel)

set(screensaver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.freedesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE(libkickoff_SRCS ${screensaver_xml} screensaver_interface)
set(krunner_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.kde.krunner.App.xml)
QT4_ADD_DBUS_INTERFACE(libkickoff_SRCS ${krunner_xml} krunner_interface)
set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(libkickoff_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_library(kickoff SHARED ${libkickoff_SRCS})
target_link_libraries(kickoff ${QT_QTCORE_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBRARY} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBS} kworkspace)
install(TARGETS kickoff ${INSTALL_TARGETS_DEFAULT_ARGS})

#######################################################################################
# Kickoff Plasma Applet

set(Applet_SRCS
    applet/applet.cpp
)
kde4_add_ui_files(Applet_SRCS applet/kickoffConfig.ui )
kde4_add_plugin(plasma_applet_launcher ${Applet_SRCS})
target_link_libraries(plasma_applet_launcher ${KDE4_PLASMA_LIBS} ${Kickoff_LIBS} ${QT_QTDECLARATIVE_LIBRARY}  ${KDE4_KIO_LIBS} kickoff )
install(TARGETS plasma_applet_launcher DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES applet/plasma-applet-launcher.desktop DESTINATION ${SERVICES_INSTALL_DIR})

#######################################################################################
# Kickoff Simple KMenu Plasma Applet

set(SimpleApplet_SRCS
    simpleapplet/contextmenufactory.cpp
    simpleapplet/menuview.cpp
    simpleapplet/simpleapplet.cpp
)
kde4_add_plugin(plasma_applet_simplelauncher ${SimpleApplet_SRCS})
target_link_libraries(plasma_applet_simplelauncher ${KDE4_KCMUTILS_LIBS} ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} ${KDE4_PLASMA_LIBS} ${Kickoff_LIBS} kworkspace kickoff)
install(TARGETS plasma_applet_simplelauncher DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES simpleapplet/plasma-applet-simplelauncher.desktop DESTINATION ${SERVICES_INSTALL_DIR})


