
set(kworkspace_LIB_SRCS kdisplaymanager.cpp
                        kwindowlistmenu.cpp
                        kworkspace.cpp
   )

if(SOPRANO_PLUGIN_RAPTORPARSER_FOUND AND SHAREDDESKTOPONTOLOGIES_ROOT_DIR)

soprano_add_ontology(
   kworkspace_LIB_SRCS
   "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig"
   "nfo"
   "Ontologies"
   "trig"
   )

soprano_add_ontology(
   kworkspace_LIB_SRCS
   "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig"
   "nco"
   "Ontologies"
   "trig"
   )

soprano_add_ontology(
   kworkspace_LIB_SRCS
   "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig"
   "pimo"
   "Ontologies"
   "trig"
   )

add_definitions(-DHAVE_SOPRANO_PLUGIN_RAPTORPARSER=1)

endif()

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_TO_ASCII)


set(ksmserver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
qt5_add_dbus_interface( kworkspace_LIB_SRCS ${ksmserver_xml} ksmserver_interface )

set(kwin_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/kwin/org.kde.KWin.xml)

set_source_files_properties(${kwin_xml} PROPERTIES INCLUDE "interface_util.h")

qt5_add_dbus_interface( kworkspace_LIB_SRCS ${kwin_xml} kwin_interface )


add_library(kworkspace SHARED ${kworkspace_LIB_SRCS})

set_target_properties(kworkspace PROPERTIES
                             VERSION 5.0.0
                             SOVERSION 5
                      )

# kdeui is needed for KWindowInfo
target_link_libraries(kworkspace Qt5::DBus Qt5::Core KF5::KI18n KF5::KWindowSystem Xau KF5::Plasma)

if(HAVE_X11)
    set_target_properties(kworkspace PROPERTIES COMPILE_FLAGS -DHAVE_X11)
    target_link_libraries(kworkspace Qt5::X11Extras ${X11_LIBRARIES})
endif()

#set_target_properties(kworkspace PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} DEFINE_SYMBOL MAKE_KWORKSPACE_LIB)
install(TARGETS kworkspace EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES kdisplaymanager.h
               kwindowlistmenu.h
               kworkspace.h
               kworkspace_export.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/kworkspace COMPONENT Devel )
