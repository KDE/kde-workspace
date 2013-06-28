project(trash)

install(DIRECTORY contents
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/trash)
install(FILES metadata.desktop
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/trash)

install(FILES metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME trash.desktop)
