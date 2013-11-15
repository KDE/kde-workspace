project(plasma-nettoolbox)


set(nettoolbox_SRCS nettoolbox.cpp)


add_library(plasma_toolbox_nettoolbox MODULE ${nettoolbox_SRCS})
target_link_libraries(plasma_toolbox_nettoolbox 
                      KF5::Plasma ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_toolbox_nettoolbox
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-toolbox-nettoolbox.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
