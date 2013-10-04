SET (ion_bbcukmet_SRCS ion_bbcukmet.cpp)
plasma_add_plugin(ion_bbcukmet ${ion_bbcukmet_SRCS})
target_link_libraries (ion_bbcukmet KF5::Solid weather_ion)

INSTALL (FILES ion-bbcukmet.desktop DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL (TARGETS ion_bbcukmet DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)

