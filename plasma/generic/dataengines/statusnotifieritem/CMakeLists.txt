project(plasma-statusnotifieritem_engine)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/statusnotifierwatcher)
include_directories(${DBusMenuQt5_INCLUDE_DIR})


# We add our source code here
set(statusnotifieritem_engine_SRCS
    statusnotifieritem_engine.cpp
    statusnotifieritemsource.cpp
    statusnotifieritemservice.cpp
    statusnotifieritemjob.cpp
    systemtraytypes.cpp
)

set(statusnotifierwatcher_xml ${KNOTIFICATIONS_DBUS_INTERFACES_DIR}/org.kde.StatusNotifierWatcher.xml)
qt5_add_dbus_interface(statusnotifieritem_engine_SRCS ${statusnotifierwatcher_xml} statusnotifierwatcher_interface)

set(statusnotifieritem_xml ${KNOTIFICATIONS_DBUS_INTERFACES_DIR}/org.kde.StatusNotifierItem.xml)

set_source_files_properties(${statusnotifieritem_xml} PROPERTIES
   NO_NAMESPACE false
   INCLUDE "systemtraytypes.h"
   CLASSNAME OrgKdeStatusNotifierItem
)
qt5_add_dbus_interface(statusnotifieritem_engine_SRCS ${statusnotifieritem_xml} statusnotifieritem_interface)

kservice_desktop_to_json(plasma-engine-statusnotifieritem.desktop)

add_library(plasma_engine_statusnotifieritem MODULE ${statusnotifieritem_engine_SRCS})
target_link_libraries(plasma_engine_statusnotifieritem
    Qt5::DBus
    KF5::KService
    KF5::Plasma
    KF5::KIconThemes
    ${DBusMenuQt5_LIBRARIES}
    KF5::KDE4Support
)

install(TARGETS plasma_engine_statusnotifieritem DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-engine-statusnotifieritem.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES statusnotifieritem.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

