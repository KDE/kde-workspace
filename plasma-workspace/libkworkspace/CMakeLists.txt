
set(kworkspace_LIB_SRCS kdisplaymanager.cpp
                        kworkspace.cpp
   )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_TO_ASCII)


set(ksmserver_xml  ${plasma-workspace_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
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

generate_export_header(kworkspace)
target_link_libraries(kworkspace Qt5::DBus Qt5::Core
                                 KF5::I18n KF5::WindowSystem KF5::Plasma
                                 Xau)
target_include_directories(kworkspace PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                                      INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/kworkspace>" )

if(X11_FOUND)
    set_target_properties(kworkspace PROPERTIES COMPILE_FLAGS -DHAVE_X11=1)
    target_link_libraries(kworkspace Qt5::X11Extras ${X11_LIBRARIES})
endif()

#set_target_properties(kworkspace PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} DEFINE_SYMBOL MAKE_KWORKSPACE_LIB)
install(TARGETS kworkspace EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES kdisplaymanager.h
               kworkspace.h
               ${CMAKE_CURRENT_BINARY_DIR}/kworkspace_export.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/kworkspace COMPONENT Devel )
