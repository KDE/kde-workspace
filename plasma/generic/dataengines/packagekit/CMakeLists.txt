project(packagekitengine)

set(packagekit_engine_SRCS
    packagekitjob.cpp
    packagekitengine.cpp
    packagekitservice.cpp
)

kde4_add_plugin(plasma_engine_packagekit ${packagekit_engine_SRCS})
target_link_libraries(plasma_engine_packagekit ${KDE4_PLASMA_LIBS} ${KDE4_KDECORE_LIBS} )

install(TARGETS plasma_engine_packagekit
    DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-dataengine-packagekit.desktop
    DESTINATION ${SERVICES_INSTALL_DIR} )

install(FILES packagekit.operations
    DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

