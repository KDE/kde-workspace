add_subdirectory( background )

include_directories( ${QIMAGEBLITZ_INCLUDES} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/background/  )

set(kcmbackgroundlib_SRCS
    background/bgdialog.cpp
    background/bgadvanced.cpp
    background/bgmonitor.cpp
    background/bgwallpaper.cpp
)
kde4_add_ui_files(kcmbackgroundlib_SRCS
    background/bgwallpaper_ui.ui
    background/bgdialog_ui.ui
    background/bgadvanced_ui.ui)

set(kcm_kdm_PART_SRCS ${kcmbackgroundlib_SRCS} ${backgroundlib_SRCS}
   background.cpp
   kdm-gen.cpp
   kdm-dlg.cpp
   kdm-theme.cpp
   kdm-shut.cpp
   kdm-users.cpp
   kdm-conv.cpp
   main.cpp
   kbackedcombobox.cpp
   positioner.cpp)


add_library(kcm_kdm MODULE ${kcm_kdm_PART_SRCS})


target_link_libraries(kcm_kdm KF5::KIOCore KF5::KIOWidgets ${QIMAGEBLITZ_LIBRARIES} KF5::NewStuff ${X11_LIBRARIES} kworkspace)

macro_add_file_dependencies(kdm-shut.cpp ${confci})
add_dependencies(kcm_kdm ConfigCi)

install(TARGETS kcm_kdm  DESTINATION ${PLUGIN_INSTALL_DIR})

#### KAuth helper and actions ####
add_executable(kcmkdmhelper helper.cpp)
target_link_libraries(kcmkdmhelper )
install(TARGETS kcmkdmhelper DESTINATION ${LIBEXEC_INSTALL_DIR})

kauth_install_helper_files(kcmkdmhelper org.kde.kcontrol.kcmkdm root)

kauth_install_actions(org.kde.kcontrol.kcmkdm kcmkdm_actions.actions)

########### install files ###############

install( FILES kdm.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

install( FILES anchor.png  DESTINATION  ${DATA_INSTALL_DIR}/kcontrol/pics )
install( FILES kdm.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )
