plasma_install_package(package org.kde.pager)

set(pager_SRCS
    plugin/model.cpp
    plugin/pager.cpp
    plugin/pagerplugin.cpp)

qt4_automoc(${pager_SRCS})
    
add_library(pagerplugin SHARED ${pager_SRCS})

target_link_libraries(pagerplugin
                      KF5::KIOCore
                      KF5::KWindowSystem
                      KF5::KActivities
                      ${QT_QTCORE_LIBRARY}
                      ${Qt5Qml_LIBRARIES}
                      ${Qt5Quick_LIBRARIES}
                      taskmanager)

install(TARGETS pagerplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/pager)
install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/pager)
