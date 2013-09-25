
set(time_engine_SRCS
    timeengine.cpp
    timesource.cpp
    solarsystem.cpp
)

kservice_desktop_to_json(plasma-dataengine-time.desktop)

plasma_add_plugin(plasma_engine_time ${time_engine_SRCS})

target_link_libraries(plasma_engine_time
    ${Solid_LIBRARIES}
    ${Plasma_LIBRARIES}
    ${KDE4Attic_LIBRARIES}
    KF5::KI18n
    KF5::KService
)

install(TARGETS plasma_engine_time DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-time.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
