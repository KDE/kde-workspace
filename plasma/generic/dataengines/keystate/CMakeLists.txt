set(keystate_engine_SRCS
    keystate.cpp
    keyservice.cpp
)

kservice_desktop_to_json(plasma-dataengine-keystate.desktop)

plasma_add_plugin(plasma_engine_keystate ${keystate_engine_SRCS})

target_link_libraries(plasma_engine_keystate
    KF5::KGuiAddons
    KF5::Plasma
    KF5::KService
    KF5::KIOCore
)

install(TARGETS plasma_engine_keystate DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-keystate.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES modifierkeystate.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

