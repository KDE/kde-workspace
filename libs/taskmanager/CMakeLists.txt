add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1204)
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY)

########### next target ###############

set(taskmanager_LIB_SRCS
   abstractgroupableitem.cpp
   abstractgroupingstrategy.cpp
   abstractsortingstrategy.cpp
   groupmanager.cpp
   launcheritem.cpp
   startup.cpp
   strategies/activitysortingstrategy.cpp
   strategies/alphasortingstrategy.cpp
   strategies/desktopsortingstrategy.cpp
   strategies/programgroupingstrategy.cpp
   strategies/manualgroupingstrategy.cpp
   strategies/manualsortingstrategy.cpp
   task.cpp
   taskactions.cpp
   taskgroup.cpp
   taskitem.cpp
   taskmanager.cpp
   tasksmodel.cpp
   launcherconfig.cpp
   launcherproperties.cpp
   )

qt5_wrap_ui(taskmanager_LIB_SRCS launcherconfig.ui launcherproperties.ui)

if(WIN32)
    set(taskmanager_LIB_SRCS ${taskmanager_LIB_SRCS} task_win.cpp)
else()
    set(taskmanager_LIB_SRCS ${taskmanager_LIB_SRCS} task_x11.cpp)
endif()

add_library(taskmanager SHARED ${taskmanager_LIB_SRCS})

target_link_libraries(taskmanager
    processcore
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::IconThemes
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets # KRun
    KF5::Service
    KF5::WindowSystem
)
if (HAVE_X11)
    target_link_libraries(taskmanager ${Qt5X11Extras_LIBRARIES} ${X11_LIBRARIES})
    if (X11_Xfixes_FOUND)
    target_link_libraries(taskmanager ${X11_Xfixes_LIB})
    endif ()
    if (X11_Xrender_FOUND)
    target_link_libraries(taskmanager ${X11_Xrender_LIB})
    endif ()
    if (X11_Xcomposite_FOUND)
    target_link_libraries(taskmanager ${X11_Xcomposite_LIB})
    endif ()
endif()

set_target_properties(taskmanager PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS taskmanager EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
   abstractgroupableitem.h
   abstractgroupingstrategy.h
   abstractsortingstrategy.h
   groupmanager.h
   launcheritem.h
   startup.h
   task.h
   taskactions.h
   taskgroup.h
   taskitem.h
   taskmanager.h
   taskmanager_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/taskmanager COMPONENT Devel
   )


#set(tasksmodel_SRCS 
#   modeltest/dynamictreemodel.cpp
#   modeltest/modeltest.cpp
#   modeltest/main.cpp
#   )

#add_executable(tasksmodeltest ${tasksmodel_SRCS})
#target_link_libraries(tasksmodeltest ${KDE4_KDEUI_LIBS} KF5::KIOCore KF5::KIOWidgets ${X11_LIBRARIES} taskmanager kephal)


