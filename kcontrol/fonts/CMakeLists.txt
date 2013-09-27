if(FONTCONFIG_FOUND)
    include_directories(${FONTCONFIG_INCLUDE_DIR})
endif(FONTCONFIG_FOUND)
include_directories(${FREETYPE_INCLUDE_DIRS})

########### next target ###############

set(kcm_fonts_PART_SRCS ../krdb/krdb.cpp fonts.cpp)

if(Q_WS_X11)
    set(kcm_fonts_PART_SRCS ${kcm_fonts_PART_SRCS} ${libkxftconfig_SRCS})
endif(Q_WS_X11)

kde4_add_plugin(kcm_fonts ${kcm_fonts_PART_SRCS})


target_link_libraries(kcm_fonts Qt5::Xml KF5::KCMUtils KF5::KI18n ${KDE4Support_LIBRARIES} ${FREETYPE_LIBRARIES})

if(Q_WS_X11)
    if(FONTCONFIG_FOUND)
        target_link_libraries(kcm_fonts ${FONTCONFIG_LIBRARIES})
    endif(FONTCONFIG_FOUND)

    target_link_libraries(kcm_fonts ${X11_LIBRARIES})
endif(Q_WS_X11)

install(TARGETS kcm_fonts  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES fonts.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
