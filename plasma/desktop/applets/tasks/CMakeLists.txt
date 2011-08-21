project(plasma-tasks-qml)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.tasks)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applets-tasks.desktop)
