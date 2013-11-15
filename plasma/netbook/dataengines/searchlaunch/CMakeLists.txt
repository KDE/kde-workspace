project(searchlaunchengine)

set(searchlaunch_engine_SRCS
        salengine.cpp
        salservice.cpp)

add_library(plasma_engine_searchlaunch MODULE ${searchlaunch_engine_SRCS})
target_link_libraries(plasma_engine_searchlaunch
                      
                      KF5::Plasma)

install(TARGETS plasma_engine_searchlaunch
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-engine-searchlaunch.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES searchlaunch.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
