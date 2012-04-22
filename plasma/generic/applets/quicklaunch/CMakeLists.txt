project(plasma-applet-quicklaunch)

set(quicklaunchplugin_SRCS
    extension/launcherlistmodel.cpp
    extension/quicklaunchhelper.cpp
    extension/quicklaunchplugin.cpp
)

qt4_automoc(${quicklaunchplugin_SRCS})

add_library(quicklaunchplugin SHARED ${quicklaunchplugin_SRCS})

target_link_libraries(
    quicklaunchplugin
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KIO_LIBS})

install(TARGETS quicklaunchplugin
        DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/quicklaunch)

install(FILES extension/qmldir
        DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/quicklaunch)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/quicklaunch)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-quicklaunch.desktop)
