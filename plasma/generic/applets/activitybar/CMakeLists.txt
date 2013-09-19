# Project Needs a name ofcourse
project(plasma-activitybar)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/activitybar)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR} RENAME activitybar.desktop)