project(analog-clock)

install(DIRECTORY contents
        DESINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/analogclock)

install(FILES metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-analogclock.desktop)
