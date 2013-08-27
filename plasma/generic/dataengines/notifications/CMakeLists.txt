set(notifications_engine_SRCS
    notificationsengine.cpp
    notificationservice.cpp
    notificationaction.cpp
)

qt4_add_dbus_adaptor( notifications_engine_SRCS org.freedesktop.Notifications.xml notificationsengine.h  NotificationsEngine )

kservice_desktop_to_json(plasma-dataengine-notifications.desktop)

plasma_add_plugin(plasma_engine_notifications ${notifications_engine_SRCS})

target_link_libraries(plasma_engine_notifications
    Qt5::DBus
    ${Plasma_LIBRARIES}
    ${KI18n_LIBRARIES}
    ${KIconThemes_LIBRARIES}
    ${KNotifications_LIBRARIES}
    ${KService_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${KDE4_KNOTIFYCONFIG_LIBRARY}
)

install(TARGETS plasma_engine_notifications DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-notifications.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES notifications.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
