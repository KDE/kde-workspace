project(nepomukcontroller-qml)

set(NepomukServiceEngine_SRCS
    nepomukserviceengine.cpp
    nepomukservicejob.cpp
    nepomukserviceservice.cpp
)

kde4_add_plugin(plasma_engine_nepomukservice ${NepomukServiceEngine_SRCS})
target_link_libraries(plasma_engine_nepomukservice ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_engine_nepomukservice DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-nepomukservice.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES nepomukservice.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
install(FILES nepomuk.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)


