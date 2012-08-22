
########### next target ###############

set(kcm_screenlocker_PART_SRCS
   scrnsave.cpp)

set(kscreenlocker_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/screenlocker/dbus/org.kde.screensaver.xml)
QT4_ADD_DBUS_INTERFACE( kcm_screenlocker_PART_SRCS ${kscreenlocker_xml} kscreensaver_interface )



kde4_add_ui_files(kcm_screenlocker_PART_SRCS screenlocker.ui )

kde4_add_plugin(kcm_screenlocker ${kcm_screenlocker_PART_SRCS})


target_link_libraries(kcm_screenlocker ${KDE4_PLASMA_LIBS} ${X11_LIBRARIES} kworkspace)

find_library(DL_LIBRARY dl)
if (DL_LIBRARY)
  target_link_libraries(kcm_screenlocker ${DL_LIBRARY})
endif(DL_LIBRARY)


install(TARGETS kcm_screenlocker  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES screenlocker.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

