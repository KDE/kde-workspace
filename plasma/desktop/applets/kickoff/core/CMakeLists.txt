set(kickoffplugin_SRCS
    applicationmodel.cpp
    favoritesmodel.cpp
    itemhandlers.cpp
    kickoffabstractmodel.cpp
    kickoffmodel.cpp
    kickoffplugin.cpp
    kickoffproxymodel.cpp
    krunnermodel.cpp
    leavemodel.cpp
    models.cpp
    recentapplications.cpp
    recentlyusedmodel.cpp
    systemmodel.cpp
    urlitemlauncher.cpp
    )

qt4_automoc(${kickoffplugin_SRCS})

qt4_add_dbus_adaptor(kickoffplugin_SRCS org.kde.kickoff.xml applicationmodel.h Kickoff::ApplicationModel)
qt4_add_dbus_adaptor(kickoffplugin_SRCS org.kde.kickoff.recent.xml recentlyusedmodel.h Kickoff::RecentlyUsedModel)

set(screensaver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.freedesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE(kickoffplugin_SRCS ${screensaver_xml} screensaver_interface)
set(krunner_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.kde.krunner.App.xml)
QT4_ADD_DBUS_INTERFACE(kickoffplugin_SRCS ${krunner_xml} krunner_interface)
set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(kickoffplugin_SRCS ${ksmserver_xml} ksmserver_interface)


add_library(kickoffplugin SHARED ${kickoffplugin_SRCS})
target_link_libraries(kickoffplugin ${QT_QTCORE_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBRARY} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBS} kworkspace)

install(TARGETS kickoffplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/kickoff)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/kickoff)
