project(plasma-tasks-qml)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.tasks)

install(FILES plasma-tasks-qml.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
