ADD_SUBDIRECTORY(ions)

SET(weather_SRCS weatherengine.cpp)
plasma_add_plugin(plasma_engine_weather ${weather_SRCS})
TARGET_LINK_LIBRARIES (plasma_engine_weather KIOCore KF5::Solid KF5::plasma weather_ion)
INSTALL (TARGETS plasma_engine_weather DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
INSTALL (FILES plasma-dataengine-weather.desktop DESTINATION ${SERVICES_INSTALL_DIR})
