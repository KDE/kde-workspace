#######################################################################################
# Kickoff Library

if(STRIGI_STRIGIQTDBUSCLIENT_LIBRARY)
    set(HAVE_STRIGIDBUS 1)
endif(STRIGI_STRIGIQTDBUSCLIENT_LIBRARY)

configure_file(core/config-kickoff-applets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kickoff-applets.h)

add_subdirectory(core)
# Package
install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/launcher)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-launcher.desktop)
