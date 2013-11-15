add_subdirectory(dashboard)

project(plasma-webapplet)

set(webapplet_SRCS
    webpage.cpp webapplet.cpp plasmawebapplet.cpp plasmajs.cpp webapplet_plugin.cpp)
add_library(plasma_appletscriptengine_webapplet MODULE ${webapplet_SRCS})
target_link_libraries(plasma_appletscriptengine_webapplet KF5::Plasma KIOCore ${QT_QTWEBKIT_LIBRARY} )
install(TARGETS plasma_appletscriptengine_webapplet DESTINATION ${PLUGIN_INSTALL_DIR})

set(webappletpackage_SRCS
    webapplet_package.cpp)
add_library(plasma_packagestructure_web MODULE ${webappletpackage_SRCS})
target_link_libraries(plasma_packagestructure_web KF5::Plasma KIOCore ${QT_QTWEBKIT_LIBRARY} )
install(TARGETS plasma_packagestructure_web DESTINATION ${PLUGIN_INSTALL_DIR})

set(dashboardapplet_SRCS
    webpage.cpp webapplet.cpp dashboardapplet.cpp bundle.cpp dashboardjs.cpp)
add_library(plasma_appletscriptengine_dashboard MODULE ${dashboardapplet_SRCS})
target_link_libraries(plasma_appletscriptengine_dashboard KF5::Plasma KIOCore ${QT_QTWEBKIT_LIBRARY} )
install(TARGETS plasma_appletscriptengine_dashboard DESTINATION ${PLUGIN_INSTALL_DIR})

set(bundlepackage_SRCS
    bundle.cpp dashboard_plugin.cpp)
add_library(plasma_packagestructure_dashboard MODULE ${bundlepackage_SRCS})
target_link_libraries(plasma_packagestructure_dashboard  KF5::Plasma KIOCore ${QT_QTWEBKIT_LIBRARY} )
install(TARGETS plasma_packagestructure_dashboard DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-scriptengine-applet-web.desktop
              plasma-scriptengine-applet-dashboard.desktop
              plasma-packagestructure-dashboard.desktop
              plasma-packagestructure-web.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
