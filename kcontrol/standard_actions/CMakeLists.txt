set(kcm_standard_actions_PART_SRCS standard_actions_module.cpp)

kde4_add_plugin(kcm_standard_actions ${kcm_standard_actions_PART_SRCS})

target_link_libraries(kcm_standard_actions
    KF5::KCMUtils
    KF5::KI18n
    KF5::XmlGui
    KF5::KDE4Support
)

install(TARGETS kcm_standard_actions DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES standard_actions.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

