set(soliddevice_engine_SRCS
    soliddeviceengine.cpp
    devicesignalmapper.cpp
    devicesignalmapmanager.cpp
    hddtemp.cpp
    soliddeviceservice.cpp
    soliddevicejob.cpp
)

kservice_desktop_to_json(plasma-dataengine-soliddevice.desktop)

plasma_add_plugin(plasma_engine_soliddevice ${soliddevice_engine_SRCS})

target_link_libraries(plasma_engine_soliddevice
  ${Plasma_LIBRARIES}
  ${Solid_LIBRARIES}
  ${QT_QTNETWORK_LIBRARY}
  ${KI18n_LIBRARIES}
  ${KDE4Attic_LIBRARIES}
  ${KDE4_KIO_LIBS}
)

install(TARGETS plasma_engine_soliddevice DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-soliddevice.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES soliddevice.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services )

