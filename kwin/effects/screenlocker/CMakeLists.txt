#######################################
# Effect

# Source files
set( kwin4_effect_builtins_sources ${kwin4_effect_builtins_sources}
    screenlocker/screenlocker.cpp
    )

# .desktop files
install( FILES
    screenlocker/screenlocker.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/kwin )

install(DIRECTORY screenlocker/qml/ DESTINATION ${DATA_INSTALL_DIR}/kwin/lockscreen)
