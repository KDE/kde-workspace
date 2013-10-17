# Project Needs a name ofcourse
project(desktoptheme)

set(kcmdesktoptheme_SRCS kcmdesktoptheme.cpp desktopthemedetails.cpp thememodel.cpp)
kde4_add_ui_files(kcmdesktoptheme_SRCS DesktopTheme.ui DesktopThemeDetails.ui)

kde4_add_plugin(kcm_desktoptheme ${kcmdesktoptheme_SRCS})
target_link_libraries(kcm_desktoptheme ${X11_LIBRARIES}
    KF5::KArchive
    KF5::KCMUtils
    KF5::KI18n
    KF5::KParts
    KF5::Plasma
    KF5::KIOCore
    KF5::KIOWidgets
    ${KDE4_KNEWSTUFF3_LIBS}
    ${KDE4Support_LIBRARIES}
)

install(TARGETS kcm_desktoptheme DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES desktoptheme.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

