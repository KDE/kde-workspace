# if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   plasma_install_package(activitybar activitybar)
   add_subdirectory(icon)
#    add_subdirectory(lock_logout)
#    add_subdirectory(panelspacer)
# endif()
#
plasma_install_package(analog-clock org.kde.analogclock)

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
    if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
        add_subdirectory(systemtray2)
    endif()
endif()
