
set(sal_SRCS
    sal.cpp
    itemcontainer.cpp
    itemview.cpp
    ../common/linearappletoverlay.cpp
    ../common/appletmovespacer.cpp
    stripwidget.cpp
    resultwidget.cpp
    runnersconfig.cpp
    iconactioncollection.cpp
    models/krunnermodel.cpp
    models/standarditemfactory.cpp
    models/favouritesmodel.cpp
    models/kservicemodel.cpp
    )

add_library(plasma_containment_sal MODULE ${sal_SRCS})
target_link_libraries(plasma_containment_sal KF5::Plasma KIOCore ${KDE4_KCMUTILS_LIBS} )

install(TARGETS plasma_containment_sal DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-sal.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-sal-menu.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

file(GLOB services services/*.desktop)
install(FILES ${services} DESTINATION ${SERVICES_INSTALL_DIR})
