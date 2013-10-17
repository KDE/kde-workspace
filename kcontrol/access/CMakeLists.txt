


########### next target ###############
find_package(Phonon4Qt5 REQUIRED NO_MODULE)
include_directories(${PHONON_INCLUDE_DIR})

set(kcm_access_PART_SRCS kcmaccess.cpp )


kde4_add_plugin(kcm_access ${kcm_access_PART_SRCS})


target_link_libraries(kcm_access
    Qt5::X11Extras
    KF5::KCMUtils
    KF5::KI18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KNotifyConfig
    KF5::KWindowSystem
    ${KDE4Support_LIBRARIES}
    ${X11_LIBRARIES}
)

install(TARGETS kcm_access  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kaccess_KDEINIT_SRCS kaccess.cpp main.cpp )


kf5_add_kdeinit_executable( kaccess ${kaccess_KDEINIT_SRCS})

target_link_libraries(kdeinit_kaccess
    Qt5::Widgets
    Qt5::X11Extras
    KF5::KConfigCore
    KF5::KCompletion
    KF5::KI18n
    KF5::KIconThemes
    KF5::KNotifications
    KF5::KWidgetsAddons
    KF5::KWindowSystem
    Phonon::phonon4qt5
    ${KDE4Support_LIBRARIES}
    ${X11_LIBRARIES}
)

install(TARGETS kdeinit_kaccess ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS kaccess         ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES kcmaccess.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kaccess.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kaccess.notifyrc  DESTINATION  ${DATA_INSTALL_DIR}/kaccess/ )
