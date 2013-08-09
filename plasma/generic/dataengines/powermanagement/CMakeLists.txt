
#kservice_desktop_to_json(plasma-dataengine-powermanagement.desktop)

set(powermanagement_engine_SRCS
    powermanagementengine.cpp
    powermanagementjob.cpp
    powermanagementservice.cpp
)

set(krunner_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.kde.krunner.App.xml)
QT4_ADD_DBUS_INTERFACE(powermanagement_engine_SRCS ${krunner_xml} krunner_interface)

plasma_add_plugin(plasma_engine_powermanagement ${powermanagement_engine_SRCS})

target_link_libraries(plasma_engine_powermanagement
    ${KCore_LIBRARIES}
    ${Solid_LIBRARIES}
    ${Plasma_LIBRARIES}
    ${KIdleTime_LIBRARIES}
) # todo: add kworkspace once ported, renable shutdown request

install(TARGETS plasma_engine_powermanagement DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-powermanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES powermanagementservice.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

