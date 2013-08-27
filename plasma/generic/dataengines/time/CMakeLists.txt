
#kservice_desktop_to_json(plasma-dataengine-time.desktop)

set(time_engine_SRCS
    timeengine.cpp
    timesource.cpp
    solarsystem.cpp
)

kservice_desktop_to_json(plasma-dataengine-time.desktop)

plasma_add_plugin(plasma_engine_time ${time_engine_SRCS})

#add_library(plasma_engine_time SHARED ${time_engine_SRCS})
target_link_libraries(plasma_engine_time
    ${Solid_LIBRARIES}
    ${Plasma_LIBRARIES}
    ${KDE4Attic_LIBRARIES}
    KF5::KI18n
    KF5::KService
)

#target_link_libraries(plasma_engine_time ${KDE4_KDECORE_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBS})


#install(TARGETS plasma_engine_time DESTINATION ${PLUGIN_INSTALL_DIR}/kplugins)
install(TARGETS plasma_engine_time DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-time.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
