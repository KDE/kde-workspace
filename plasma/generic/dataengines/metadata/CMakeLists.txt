project(metadata)

set(metadata_engine_SRCS metadata_engine.cpp)
 
plasma_add_plugin(plasma_engine_metadata ${metadata_engine_SRCS})

include_directories(${SOPRANO_INCLUDE_DIR} ${NEPOMUK_CORE_INCLUDE_DIR})

target_link_libraries(plasma_engine_metadata
		      ${KDE4_KIO_LIBS}
                      KF5::KCoreAddons
                      KF5::plasma
                      ${NEPOMUK_CORE_LIBRARY}
		      ${SOPRANO_LIBRARIES} )
                      
install(TARGETS plasma_engine_metadata
        DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
 
install(FILES plasma-engine-metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
