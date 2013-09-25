set(apps_engine_SRCS
    appsengine.cpp
    appsource.cpp
    appservice.cpp
    appjob.cpp
)

kservice_desktop_to_json(plasma-dataengine-apps.desktop)

plasma_add_plugin(plasma_engine_apps ${apps_engine_SRCS})

target_link_libraries(plasma_engine_apps
    ${Plasma_LIBRARIES}
    ${KService_LIBRARIES}
    ${KDE4_KIO_LIBS} #FIXME WHEN KRUN MOVES TO KF5
)

install(TARGETS plasma_engine_apps DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-apps.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES apps.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
