project (autostart)

include (KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

########### next target ###############

set(kcm_autostart_PART_SRCS
   autostartitem.cpp
   addscriptdialog.cpp
   advanceddialog.cpp
   autostart.cpp )

kde4_add_ui_files(kcm_autostart_PART_SRCS autostartconfig.ui )

add_library(kcm_autostart MODULE ${kcm_autostart_PART_SRCS})

target_link_libraries(kcm_autostart KF5::KCMUtils KF5::KI18n KF5::KIOCore KF5::KIOWidgets)

########### install files ###############

install(TARGETS kcm_autostart  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES autostart.desktop   DESTINATION  ${SERVICES_INSTALL_DIR} )
