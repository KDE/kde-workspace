project(packagekitengine)

set(packagekit_engine_SRCS
    packagekitjob.cpp
    packagekitengine.cpp
    packagekitservice.cpp
)

plasma_add_plugin(plasma_engine_packagekit ${packagekit_engine_SRCS})
target_link_libraries(plasma_engine_packagekit KF5::plasma KF5::KCoreAddons )

install(TARGETS plasma_engine_packagekit
    DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)

install(FILES plasma-dataengine-packagekit.desktop
    DESTINATION ${SERVICES_INSTALL_DIR} )

install(FILES packagekit.operations
    DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

