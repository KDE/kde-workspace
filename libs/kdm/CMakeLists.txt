
include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/kdm/kfrontend  )

########### next target ###############

set(kgreet_classic_PART_SRCS kgreet_classic.cpp )


add_library(kgreet_classic MODULE ${kgreet_classic_PART_SRCS})


target_link_libraries(kgreet_classic  ${KDE4_KDEUI_LIBS})

install(TARGETS kgreet_classic  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kgreet_winbind_PART_SRCS kgreet_winbind.cpp )


add_library(kgreet_winbind MODULE ${kgreet_winbind_PART_SRCS})


target_link_libraries(kgreet_winbind ${KDE4_KDEUI_LIBS})

install(TARGETS kgreet_winbind  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############

set(kgreet_generic_PART_SRCS kgreet_generic.cpp )


add_library(kgreet_generic MODULE ${kgreet_generic_PART_SRCS})


target_link_libraries(kgreet_generic ${KDE4_KDEUI_LIBS})

install(TARGETS kgreet_generic  DESTINATION ${PLUGIN_INSTALL_DIR} )





########### install files ###############

install( FILES kgreeterplugin.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )
