project(plasma-containmentactions-switchactivity)
include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)

set(switchactivity_SRCS
    switch.cpp
)

add_library(plasma_containmentactions_switchactivity MODULE ${switchactivity_SRCS})
target_link_libraries(plasma_containmentactions_switchactivity KF5::Plasma KIOCore kworkspace)

install(TARGETS plasma_containmentactions_switchactivity DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-switchactivity.desktop DESTINATION ${SERVICES_INSTALL_DIR})
