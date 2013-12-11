set(apps_engine_SRCS
    appsengine.cpp
    appsource.cpp
    appservice.cpp
    appjob.cpp
)

kservice_desktop_to_json(plasma-dataengine-apps.desktop)

add_library(plasma_engine_apps MODULE ${apps_engine_SRCS})

target_link_libraries(plasma_engine_apps
    KF5::Plasma
    KF5::Service
    KF5::KIOCore
    KF5::KIOWidgets
)

install(TARGETS plasma_engine_apps DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-apps.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES apps.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
