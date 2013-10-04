SET (ion_noaa_SRCS ion_noaa.cpp)
plasma_add_plugin(ion_noaa ${ion_noaa_SRCS})
target_link_libraries (ion_noaa KF5::Solid weather_ion)

INSTALL (FILES ion-noaa.desktop DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL (TARGETS ion_noaa DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)

