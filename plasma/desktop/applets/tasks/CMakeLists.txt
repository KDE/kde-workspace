project(plasma-tasks)

set(tasks_SRCS
    tasks.cpp
    support/draghelper.cpp
    support/textlabel.cpp
    support/declarativeitemcontainer.cpp
    support/tooltip.cpp)

kde4_add_ui_files(tasks_SRCS tasksConfig.ui)

kde4_add_plugin(plasma_applet_tasks ${tasks_SRCS})

target_link_libraries(plasma_applet_tasks ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${QT_QTDECLARATIVE_LIBRARY} taskmanager)

install(TARGETS plasma_applet_tasks DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-tasks-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.plasma.tasks)
