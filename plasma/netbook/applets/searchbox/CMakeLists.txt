project(plasma-searchbox)


set(searchbox_SRCS searchbox.cpp)


add_library(plasma_applet_searchbox MODULE ${searchbox_SRCS})
target_link_libraries(plasma_applet_searchbox 
                      KF5::Plasma ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_searchbox
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-searchbox.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
