add_subdirectory(kickoff)
plasma_install_package(trash org.kde.trash plasma/plasmoids applet)

if(NOT WIN32)
   # tasks and windowlist depend on libs/taskmanager
   add_subdirectory(taskmanager)
   plasma_install_package(window-list org.kde.windowlist plasma/plasmoids applet)

if(KActivities_FOUND)
   add_subdirectory(pager)
   add_subdirectory(showActivityManager)
endif()
endif()
