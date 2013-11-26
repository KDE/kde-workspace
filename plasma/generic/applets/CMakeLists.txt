plasma_install_package(activitybar activitybar)
add_subdirectory(icon)
#    add_subdirectory(lock_logout)
#    add_subdirectory(panelspacer)
#
plasma_install_package(analog-clock org.kde.analogclock)
plasma_install_package(mediacontroller org.kde.mediacontroller)

add_subdirectory(batterymonitor)
# add_subdirectory(calendar)
add_subdirectory(devicenotifier)
plasma_install_package(digital-clock org.kde.digitalclock)
plasma_install_package(calendar org.kde.calendar)
# add_subdirectory(quicklaunch)
#
# if(QT_QTWEBKIT_FOUND)
#    add_subdirectory(webbrowser)
# endif()
#
if(NOT WIN32)
#    add_subdirectory(system-monitor)
#    #notifications
#    #should compile also on windows? (even if doesn't really make sense)
    add_subdirectory(notifications)
    add_subdirectory(systemtray2)
endif()
