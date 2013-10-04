SET (ion_envcan_SRCS ion_envcan.cpp)
plasma_add_plugin(ion_envcan ${ion_envcan_SRCS})
target_link_libraries (ion_envcan KF5::Solid weather_ion)

INSTALL (FILES ion-envcan.desktop DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL (TARGETS ion_envcan DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)

