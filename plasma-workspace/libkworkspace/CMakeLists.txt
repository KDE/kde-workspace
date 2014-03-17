
set(kworkspace_LIB_SRCS kdisplaymanager.cpp
                        kworkspace.cpp
   )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_TO_ASCII)


set(ksmserver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/plasma-workspace/ksmserver/org.kde.KSMServerInterface.xml)
qt5_add_dbus_interface( kworkspace_LIB_SRCS ${ksmserver_xml} ksmserver_interface )

set(kwin_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/kwin/org.kde.KWin.xml)

set_source_files_properties(${kwin_xml} PROPERTIES INCLUDE "interface_util.h")

qt5_add_dbus_interface( kworkspace_LIB_SRCS ${kwin_xml} kwin_interface )


add_library(kworkspace ${kworkspace_LIB_SRCS})
add_library(PW::KWorkspace ALIAS kworkspace)
set_target_properties(kworkspace PROPERTIES
                             VERSION 5.0.0
                             SOVERSION 5
                      )

# kdeui is needed for KWindowInfo
target_link_libraries(kworkspace Qt5::DBus Qt5::Core KF5::I18n KF5::WindowSystem Xau KF5::Plasma)
target_include_directories(kworkspace PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                                      INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/kworkspace>" )

if(HAVE_X11)
    set_target_properties(kworkspace PROPERTIES COMPILE_FLAGS -DHAVE_X11)
    target_link_libraries(kworkspace Qt5::X11Extras ${X11_LIBRARIES})
endif()

#set_target_properties(kworkspace PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} DEFINE_SYMBOL MAKE_KWORKSPACE_LIB)
install(TARGETS kworkspace EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES kdisplaymanager.h
               kworkspace.h
               kworkspace_export.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/kworkspace COMPONENT Devel )
