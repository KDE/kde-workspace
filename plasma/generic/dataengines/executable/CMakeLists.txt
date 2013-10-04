set(executable_engine_SRCS
    executable.cpp
)

kservice_desktop_to_json(plasma-dataengine-executable.desktop)

plasma_add_plugin(plasma_engine_executable ${executable_engine_SRCS})

target_link_libraries(plasma_engine_executable
    ${Plasma_LIBRARIES}
    KF5::KService
    KF5::KCoreAddons
)

install(TARGETS plasma_engine_executable DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-executable.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
