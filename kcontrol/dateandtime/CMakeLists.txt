########### next target ###############

set(kcm_clock_PART_SRCS dtime.cpp main.cpp )

kde4_add_ui_files(kcm_clock_PART_SRCS dateandtime.ui)
kde4_add_plugin(kcm_clock ${kcm_clock_PART_SRCS})

target_link_libraries(kcm_clock
    Qt5::DBus
    KF5::ItemViews
    KF5::KAuth
    KF5::KCMUtils
    KF5::KI18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Plasma
    ${KDE4Support_LIBRARIES}
)

install(TARGETS kcm_clock  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############

kde4_add_executable(kcmdatetimehelper helper.cpp ${helper_mocs})
target_link_libraries(kcmdatetimehelper KF5::KAuth KF5::KConfigCore ${KDE4Support_LIBRARIES})
install(TARGETS kcmdatetimehelper DESTINATION ${LIBEXEC_INSTALL_DIR})

kde4_install_auth_helper_files(kcmdatetimehelper org.kde.kcontrol.kcmclock root)

kde4_install_auth_actions(org.kde.kcontrol.kcmclock kcmclock_actions.actions)

########### install files ###############

install( FILES clock.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
