project(plasma-tasks)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.tasks)

install(FILES plasma-tasks-default.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
