project(metadata)

set(metadata_engine_SRCS metadata_engine.cpp)
 
add_library(plasma_engine_metadata MODULE ${metadata_engine_SRCS})

include_directories(${SOPRANO_INCLUDE_DIR} ${NEPOMUK_CORE_INCLUDE_DIR})

target_link_libraries(plasma_engine_metadata
		      KIOCore
                      KF5::CoreAddons
                      KF5::Plasma
                      ${NEPOMUK_CORE_LIBRARY}
		      ${SOPRANO_LIBRARIES} )
                      
install(TARGETS plasma_engine_metadata
        DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
 
install(FILES plasma-engine-metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
