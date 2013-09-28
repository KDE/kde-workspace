set(systemtrayplugin_SRCS
    systemtrayplugin.cpp
    host.cpp
    manager.cpp
    protocol.cpp
    task.cpp

    protocols/dbussystemtray/dbussystemtraytask.cpp
    protocols/dbussystemtray/dbussystemtrayprotocol.cpp

)

add_library(systemtrayplugin SHARED ${systemtrayplugin_SRCS})

target_link_libraries(systemtrayplugin
    ${Qt5Quick_LIBRARIES}
    ${Qt5Qml_LIBRARIES}
    ${KCore_LIBRARIES}
    ${KDECLARATIVE_LIBRARIES}
    ${Plasma_LIBRARIES}
    KF5::KWindowSystem
    ${KIconThemes_LIBRARIES}

    ${KDE4Attic_LIBRARIES}
    ${KDE4Support_LIBRARIES}
    )

set(plugin_install_path ${QML_INSTALL_DIR}/org/kde/private/systemtray)

install(TARGETS systemtrayplugin DESTINATION ${plugin_install_path})
install(FILES qmldir DESTINATION ${plugin_install_path})

