project(trash)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/trash)
install(FILES package/trash.desktop
        DESTINATION ${DATA_INSTALL_DIR}/usr/share/kde4/services/trash.desktop)

install(FILES package/trash.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        )
