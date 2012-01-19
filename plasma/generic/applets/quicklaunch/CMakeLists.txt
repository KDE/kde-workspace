project(plasma-applet-quicklaunch)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/quicklaunch)

install(FILES package/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-quicklaunch.desktop)
