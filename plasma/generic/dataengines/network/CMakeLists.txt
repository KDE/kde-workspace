set(network_engine_SRCS
    networkengine.cpp
)

plasma_add_plugin(plasma_engine_network ${network_engine_SRCS})
target_link_libraries(plasma_engine_network KF5::KCoreAddons KF5::Plasma solidcontrol)

install(TARGETS plasma_engine_network DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-network.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

