set(filebrowser_engine_SRCS
    filebrowserengine.cpp
)

kservice_desktop_to_json(plasma-dataengine-filebrowser.desktop)

plasma_add_plugin(plasma_engine_filebrowser ${filebrowser_engine_SRCS})
target_link_libraries(plasma_engine_filebrowser
    ${Plasma_LIBRARIES}
    ${KService_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${KDE4Support_LIBRARIES}
)

install(TARGETS plasma_engine_filebrowser DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-filebrowser.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

