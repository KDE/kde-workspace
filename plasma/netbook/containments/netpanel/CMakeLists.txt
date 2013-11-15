
set(panel_SRCS
    panel.cpp
    dummytoolbox.cpp
    ../common/linearappletoverlay.cpp
    ../common/appletmovespacer.cpp
    )

add_library(plasma_containment_netpanel MODULE ${panel_SRCS})
target_link_libraries(plasma_containment_netpanel KF5::Plasma KIOCore)

install(TARGETS plasma_containment_netpanel DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-netpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR})
