set(keystate_engine_SRCS
    keystate.cpp
    keyservice.cpp
)

kservice_desktop_to_json(plasma-dataengine-keystate.desktop)

plasma_add_plugin(plasma_engine_keystate ${keystate_engine_SRCS})

target_link_libraries(plasma_engine_keystate
  ${KGuiAddons_LIBRARIES}
  ${Plasma_LIBRARIES}
  ${KService_LIBRARIES}
  ${KDE4_KIO_LIBS}
)

install(TARGETS plasma_engine_keystate DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-keystate.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES modifierkeystate.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

