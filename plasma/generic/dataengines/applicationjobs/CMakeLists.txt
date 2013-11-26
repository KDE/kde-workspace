project(kuiserver_engine)

#include (KDE4Defaults)
#include (MacroLibrary)

set(kuiserver_engine_SRCS
    kuiserverengine.cpp
    jobcontrol.cpp
    jobaction.cpp
)

qt5_add_dbus_adaptor(kuiserver_engine_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.JobViewV2.xml kuiserverengine.h JobView jobviewadaptor )
qt5_add_dbus_adaptor(kuiserver_engine_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.JobViewServer.xml kuiserverengine.h KuiserverEngine jobviewserveradaptor )

kservice_desktop_to_json(plasma-dataengine-applicationjobs.desktop)

add_library(plasma_engine_applicationjobs MODULE ${kuiserver_engine_SRCS})
target_link_libraries(plasma_engine_applicationjobs
  Qt5::DBus
  KF5::KI18n
  KF5::KIOCore
  KF5::Plasma
  KF5::KService
  KF5::KDE4Support
)

install(TARGETS plasma_engine_applicationjobs DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-applicationjobs.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES applicationjobs.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

