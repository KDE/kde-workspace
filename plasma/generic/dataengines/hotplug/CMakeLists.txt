set(hotplug_engine_SRCS
    hotplugengine.cpp
    hotplugservice.cpp
    hotplugjob.cpp
)

kservice_desktop_to_json(plasma-dataengine-hotplug.desktop)

plasma_add_plugin(plasma_engine_hotplug ${hotplug_engine_SRCS})
target_link_libraries(plasma_engine_hotplug
                Qt5::DBus
                ${KCore_LIBRARIES}
                ${Plasma_LIBRARIES}
                ${Solid_LIBRARIES}
                ${KService_LIBRARIES}
                ${KDE4_KIO_LIBS} #FIXME WHEN KRUN MOVES TO KF5
                )

install(TARGETS plasma_engine_hotplug DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-hotplug.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES hotplug.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services )
