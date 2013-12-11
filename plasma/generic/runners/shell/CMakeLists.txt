set(krunner_shell_SRCS
    shellrunner.cpp
    shell_config.cpp
)

kde4_add_ui_files(krunner_shell_SRCS shellOptions.ui)
add_library(krunner_shell MODULE ${krunner_shell_SRCS})
target_link_libraries(krunner_shell KIOCore KF5::Plasma KF5::KF5Su)

install(TARGETS krunner_shell DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-shell.desktop DESTINATION ${SERVICES_INSTALL_DIR})

