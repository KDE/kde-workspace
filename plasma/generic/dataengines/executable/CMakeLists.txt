set(executable_engine_SRCS
    executable.cpp
)

kservice_desktop_to_json(plasma-dataengine-executable.desktop)

add_library(plasma_engine_executable MODULE ${executable_engine_SRCS})

target_link_libraries(plasma_engine_executable
    KF5::Plasma
    KF5::Service
    KF5::CoreAddons
)

install(TARGETS plasma_engine_executable DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-executable.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
