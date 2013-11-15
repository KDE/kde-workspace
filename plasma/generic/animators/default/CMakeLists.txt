set(default_animator_SRCS
    defaultAnimator.cpp
)

add_library(plasma_animator_default MODULE ${default_animator_SRCS})
target_link_libraries(plasma_animator_default ${KDE4_KDEUI_LIBS} KF5::Plasma)

install(TARGETS plasma_animator_default DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-animator-default.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

