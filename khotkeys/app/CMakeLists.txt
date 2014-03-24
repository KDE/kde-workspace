########### next target ###############

set(kded_khotkeys_PART_SRCS 
        kded.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.khotkeys.xml)

qt5_generate_dbus_interface(kded.h org.kde.khotkeys.xml )

add_library(kded_khotkeys MODULE ${kded_khotkeys_PART_SRCS})
target_link_libraries(kded_khotkeys
    khotkeysprivate
    PW::KWorkspace
    KF5::DBusAddons
    KF5::KDE4Support)

# I'm not sure if this is needed anymore. Check it
if(X11_XTest_FOUND)
    target_link_libraries(kded_khotkeys ${X11_XTest_LIB})
endif()

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KHotKeysDBusInterface")

ecm_configure_package_config_file(KHotKeysDBusInterfaceConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/KHotKeysDBusInterfaceConfig.cmake
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR})

########### install files ###############
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KHotKeysDBusInterfaceConfig.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(TARGETS kded_khotkeys  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES khotkeys.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/kded)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.khotkeys.xml
        DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})


