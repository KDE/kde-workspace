set(systemtrayplugin_SRCS
    systemtrayplugin.cpp
    host.cpp
    manager.cpp
    protocol.cpp
    task.cpp

    protocols/dbussystemtray/dbussystemtraytask.cpp
    protocols/dbussystemtray/dbussystemtrayprotocol.cpp

    protocols/plasmoid/plasmoidtask.cpp
    protocols/plasmoid/plasmoidprotocol.cpp
    protocols/plasmoid/plasmoidinterface.cpp
)

add_library(systemtrayplugin SHARED ${systemtrayplugin_SRCS})

target_link_libraries(systemtrayplugin
    Qt5::Quick
    Qt5::Qml
    KF5::KDeclarative
    KF5::Plasma
    KF5::PlasmaQuick
    KF5::KWindowSystem
    KF5::KService
    KF5::KI18n
    KF5::KIconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KDE4Support
    )

set(plugin_install_path ${QML_INSTALL_DIR}/org/kde/private/systemtray)

install(TARGETS systemtrayplugin DESTINATION ${plugin_install_path})
install(FILES qmldir DESTINATION ${plugin_install_path})

