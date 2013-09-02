add_subdirectory(kickoff)
#add_subdirectory(showActivityManager)
plasma_install_package(trash org.kde.trash plasma/plasmoids applet)

if(NOT WIN32)
    #task and windowlist depend on libs/taskmanager
   #add_subdirectory(tasks)
   plasma_install_package(window-list org.kde.windowlist plasma/plasmoids applet)

   #add_subdirectory(pager)
endif(NOT WIN32)
