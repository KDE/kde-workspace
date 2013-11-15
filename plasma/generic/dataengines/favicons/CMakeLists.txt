set(favicons_engine_SRCS
    favicons.cpp
    faviconprovider.cpp
)

add_library(plasma_engine_favicons MODULE ${favicons_engine_SRCS} )
target_link_libraries(plasma_engine_favicons KF5::Plasma ${KDE4_KDEUI_LIBS} KIOCore)

install(TARGETS plasma_engine_favicons DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine )
install(FILES plasma-dataengine-favicons.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


