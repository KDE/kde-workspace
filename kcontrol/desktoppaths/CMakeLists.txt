set(kcm_desktoppaths_PART_SRCS
   globalpaths.cpp
)

kde4_add_plugin(kcm_desktoppaths ${kcm_desktoppaths_PART_SRCS})

target_link_libraries(kcm_desktoppaths
    KF5::KCMUtils
    KF5::KI18n
    KF5::KIOCore
    KF5::KIOWidgets
    ${KDE4Support_LIBRARIES}
)

install(TARGETS kcm_desktoppaths DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES desktoppath.desktop DESTINATION ${SERVICES_INSTALL_DIR})

