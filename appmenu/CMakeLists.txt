include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    )
include_directories(${DBUSMENUQT_INCLUDE_DIR})

set(kded_appmenu_SRCS
    appmenu.cpp
    menuimporter.cpp
    appmenu_dbus.cpp
    menubutton.cpp
    menuwidget.cpp
    menubar.cpp
    topmenubar.cpp
    glowbar.cpp
    verticalmenu.cpp
    )

qt4_add_dbus_adaptor(kded_appmenu_SRCS com.canonical.AppMenu.Registrar.xml
    menuimporter.h MenuImporter menuimporteradaptor MenuImporterAdaptor)

qt4_add_dbus_adaptor(kded_appmenu_SRCS org.kde.kded.appmenu.xml
    appmenu_dbus.h AppmenuDBus appmenuadaptor AppmenuAdaptor)

kde4_add_plugin(kded_appmenu ${kded_appmenu_SRCS})

target_link_libraries(kded_appmenu  ${KDE4_KIO_LIBS} ${X11_LIBRARIES} ${DBUSMENUQT_LIBRARIES} ${KDE4_PLASMA_LIBS})

install(TARGETS kded_appmenu  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES appmenu.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )
install( FILES com.canonical.AppMenu.Registrar.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
install( FILES org.kde.kded.appmenu.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
