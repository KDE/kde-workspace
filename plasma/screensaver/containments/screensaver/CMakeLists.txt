set(desktop_SRCS
    desktop.cpp
    ../../../desktop/toolboxes/internaltoolbox.cpp)

add_library(plasma_containment_saverdesktop MODULE ${desktop_SRCS})
target_link_libraries(plasma_containment_saverdesktop ${KDE4_KDEUI_LIBS} KF5::Plasma)

install(TARGETS plasma_containment_saverdesktop DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-saverdesktop.desktop DESTINATION ${SERVICES_INSTALL_DIR})

