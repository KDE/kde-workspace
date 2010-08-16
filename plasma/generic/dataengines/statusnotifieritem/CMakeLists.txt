project(plasma-statusnotifieritem_engine)

if ( DBUSMENUQT_FOUND )
   set( _plasma_DBUSMENUQT_INCLUDE_DIR ${DBUSMENUQT_INCLUDE_DIR} )
   set( _plasma_DBUSMENUQT_LIBRARIES ${DBUSMENUQT_LIBRARIES} )
else ( DBUSMENUQT_FOUND )
   set( _plasma_DBUSMENUQT_INCLUDE_DIR )
   set( _plasma_DBUSMENUQT_LIBRARIES )
endif ( DBUSMENUQT_FOUND )

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/statusnotifierwatcher)
include_directories(${_plasma_DBUSMENUQT_INCLUDE_DIR})

# We add our source code here
set(statusnotifieritem_engine_SRCS
    statusnotifieritem_engine.cpp
    statusnotifieritemsource.cpp
    statusnotifieritemservice.cpp
    statusnotifieritemjob.cpp
    systemtraytypes.cpp
)

set(statusnotifierwatcher_xml ${KDE4_DBUS_INTERFACES_DIR}/org.kde.StatusNotifierWatcher.xml)
QT4_ADD_DBUS_INTERFACE(statusnotifieritem_engine_SRCS ${statusnotifierwatcher_xml} statusnotifierwatcher_interface)

set(statusnotifieritem_xml ${KDE4_DBUS_INTERFACES_DIR}/org.kde.StatusNotifierItem.xml)

set_source_files_properties(${statusnotifieritem_xml} PROPERTIES
   NO_NAMESPACE false
   INCLUDE "systemtraytypes.h"
   CLASSNAME OrgKdeStatusNotifierItem
)
QT4_ADD_DBUS_INTERFACE(statusnotifieritem_engine_SRCS ${statusnotifieritem_xml} statusnotifieritem_interface)

kde4_add_plugin(plasma_engine_statusnotifieritem ${statusnotifieritem_engine_SRCS})
target_link_libraries(plasma_engine_statusnotifieritem ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${_plasma_DBUSMENUQT_LIBRARIES})

install(TARGETS plasma_engine_statusnotifieritem DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma_engine_statusnotifieritem.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES statusnotifieritem.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

