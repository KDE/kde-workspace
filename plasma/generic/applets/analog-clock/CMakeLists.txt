project(analog-clock)

install(DIRECTORY package/
        DESINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/notifier)

install(FILES pacckage/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-analogclock.desktop)
