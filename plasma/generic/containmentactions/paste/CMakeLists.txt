project(plasma-containmentactions-paste)

set(paste_SRCS
    paste.cpp
)

kservice_desktop_to_json(plasma-containmentactions-paste.desktop)


add_library(plasma_containmentactions_paste MODULE ${paste_SRCS})
target_link_libraries(plasma_containmentactions_paste 
	KF5::Plasma 
	KF5::KIOCore
	)

install(TARGETS plasma_containmentactions_paste DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-paste.desktop DESTINATION ${SERVICES_INSTALL_DIR})
