


set(device_notifications_engine_SRCS
    devicenotificationsengine.cpp
)

qt4_add_dbus_adaptor( device_notifications_engine_SRCS org.kde.DeviceNotifications.xml devicenotificationsengine.h DeviceNotificationsEngine )

#kde4_add_plugin(plasma_engine_devicenotifications ${device_notifications_engine_SRCS})

add_library(plasma_engine_devicenotifications MODULE ${device_notifications_engine_SRCS} )
set_target_properties(plasma_engine_devicenotifications PROPERTIES PREFIX "")

target_link_libraries(plasma_engine_devicenotifications 
    ${KCore_LIBRARIES} 
    #${Solid_LIBRARIES} 
    ${Plasma_LIBRARIES} 
    #${KDE4_KIDLETIME_LIBRARY} 
) # todo: add kworkspace once ported

#target_link_libraries(plasma_engine_devicenotifications ${KDE4_PLASMA_LIBS} ${KDE4_KDECORE_LIBS})

install(TARGETS plasma_engine_devicenotifications DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-devicenotifications.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
