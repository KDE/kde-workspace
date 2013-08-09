set(places_engine_SRCS
    placesengine.cpp
    placeservice.cpp
    setupdevicejob.cpp
    modeljob.h
)

set(CMAKE_AUTOMOC TRUE)

kservice_desktop_to_json(plasma-dataengine-places.desktop)

plasma_add_plugin(plasma_engine_places ${places_engine_SRCS})

target_link_libraries(plasma_engine_places
	${Plasma_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${KDE4_KFILE_LIBS}
    ${Solid_LIBRARIES}
    ${KDE4Support_LIBRARIES}
)

install(TARGETS plasma_engine_places
        DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-places.desktop
        DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES org.kde.places.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services )

