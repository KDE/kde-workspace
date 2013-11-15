project(plasma-panelspacer-internal)

set(panelspacer_SRCS
    panelspacer.cpp)

add_library(plasma_applet_panelspacer_internal MODULE ${panelspacer_SRCS})
target_link_libraries(plasma_applet_panelspacer_internal KF5::Plasma)

install(TARGETS plasma_applet_panelspacer_internal DESTINATION ${PLUGIN_INSTALL_DIR})



install(FILES plasma-applet-panelspacer-internal.desktop DESTINATION ${SERVICES_INSTALL_DIR})

