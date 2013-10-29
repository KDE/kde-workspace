
set(kcm_colors_SRCS ../krdb/krdb.cpp colorscm.cpp previewwidget.cpp setpreviewwidget.cpp)

set(klauncher_xml ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KLauncher.xml)
qt5_add_dbus_interface(kcm_colors_SRCS ${klauncher_xml} klauncher_iface)

kde4_add_ui_files(kcm_colors_SRCS colorsettings.ui preview.ui setpreview.ui)

kde4_add_plugin(kcm_colors ${kcm_colors_SRCS})
target_link_libraries(kcm_colors
    KF5::KCMUtils
    KF5::KGuiAddons
    KF5::KI18n
    Qt5::DBus
    ${KDE4_KNEWSTUFF3_LIBS}
    KF5::KDE4Support
    ${X11_LIBRARIES})

install(TARGETS kcm_colors  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES colors.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES colorschemes.knsrc DESTINATION  ${CONFIG_INSTALL_DIR} )

# built-in color schemes
FILE(GLOB schemefiles schemes/*.colors)
install( FILES ${schemefiles} DESTINATION ${DATA_INSTALL_DIR}/color-schemes )
