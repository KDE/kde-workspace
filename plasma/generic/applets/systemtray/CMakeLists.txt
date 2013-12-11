project(plasma-systemtray)
#TODO: see if is still the case
# 'engineName' causes error
#kde4_no_enable_final(plasma-systemtray)

#find_package(Qt4 4.8 COMPONENTS QtCore QtGui QtDeclarative REQUIRED)

set(data_install_dir "plasma/packages/org.kde.systemtray")

set(systemtray_SRCS

    core/manager.cpp
    core/protocol.cpp
    core/task.cpp

#    protocols/fdo/fdoprotocol.cpp
#     protocols/fdo/fdotask.cpp
#     #FIXME: we should find another way to achieve that
#     #protocols/fdo/fdonotification.cpp
#     protocols/fdo/fdographicswidget.cpp
#    protocols/fdo/fdoselectionmanager.cpp
#     protocols/fdo/x11embedcontainer.cpp
#     protocols/fdo/x11embeddelegate.cpp
#     protocols/fdo/x11embedpainter.cpp
#
    protocols/plasmoid/plasmoidtaskprotocol.cpp
    protocols/plasmoid/plasmoidtask.cpp

    protocols/dbussystemtray/dbussystemtraytask.cpp
    protocols/dbussystemtray/dbussystemtrayprotocol.cpp

    ui/applet.cpp
    ui/widgetitem.cpp
    ui/mouseredirectarea.cpp
    )

kde4_add_ui_files(systemtray_SRCS
                  ui/autohide.ui
                  ui/visibleitems.ui)

#include(${QT_USE_FILE})
include (CheckLibraryExists)
check_library_exists (Xss XScreenSaverQueryInfo "" HAVE_LIBXSS)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

if (HAVE_LIBXSS)
   set (IDLE_DETECTION_LIB "Xss")
endif ()

add_library(plasma_applet_systemtray MODULE ${systemtray_SRCS})
include_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(plasma_applet_systemtray
    Qt5::DBus
    Qt5::Quick
    KF5::Plasma
    KF5::I18n
    KF5::ConfigWidgets
    KF5::IconThemes
    KF5::Notifications
    KF5::Service
    KF5::Solid
    KF5::WindowSystem
    KF5::KIOCore
    KF5::KDE4Support
    ${X11_LIBRARIES}
    ${X11_Xrender_LIB}
    ${X11_Xfixes_LIB}
    ${X11_Xdamage_LIB}
    ${X11_Xcomposite_LIB}
#    ${IDLE_DETECTION_LIB}
)

#install(TARGETS plasma_applet_systemtray DESTINATION ${PLUGIN_INSTALL_DIR})
#install(DIRECTORY "package/" DESTINATION "${DATA_INSTALL_DIR}/${data_install_dir}")
#install(FILES plasma-applet-systemtray.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(systemtrayplugin_SRCS
    core/systemtrayplugin.cpp
    )

add_library(systemtrayplugin SHARED ${systemtrayplugin_SRCS})

target_link_libraries(systemtrayplugin
    Qt5::Quick
    Qt5::Qml
    ${KCore_LIBRARIES}
    ${KDECLARATIVE_LIBRARIES}
    KF5::Plasma
    KF5::WindowSystem
    )

install(TARGETS systemtrayplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/systemtray)
#install(FILES qmldir ToolTip.qml DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/core)
