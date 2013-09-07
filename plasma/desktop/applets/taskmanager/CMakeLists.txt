plasma_install_package(package org.kde.plasma.taskmanager)

set(taskmanagerplugin_SRCS
    plugin/backend.cpp
    plugin/draghelper.cpp
    plugin/taskmanagerplugin.cpp
    plugin/textlabel.cpp
)

qt4_automoc(${taskmanagerplugin_SRCS})

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/taskmanager)

add_library(taskmanagerplugin SHARED ${taskmanagerplugin_SRCS})

target_link_libraries(taskmanagerplugin
                      taskmanager
                      ${QT_QTCORE_LIBRARY}
                      ${Qt5Qml_LIBRARIES}
                      ${Qt5Quick_LIBRARIES}
                      ${Qt5Widgets_LIBRARIES}
                      KF5::KIOCore
                      KF5::KWindowSystem)

install(TARGETS taskmanagerplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/taskmanager)
