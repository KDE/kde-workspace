project(nepomukcontroller-qml)



install(DIRECTORY contents
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/nepomukcontroller-qml)

install(FILES metadata.desktop
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/nepomukcontroller-qml)

install(FILES metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR} RENAME nepomukcontroller-qml.desktop)

# install update script to replace the old nepomuk-controller with this nepomukcontroller-qml
install(FILES replace-old-nepomukcontroller-with-qml.js
        DESTINATION ${DATA_INSTALL_DIR}/plasma-desktop/updates/)