add_subdirectory(tests)

set(appletbrowserdialog_SRCS
    widgetsexplorer/kcategorizeditemsviewmodels.cpp
    widgetsexplorer/plasmaappletitemmodel.cpp
    widgetsexplorer/widgetexplorer.cpp
    widgetsexplorer/openwidgetassistant.cpp
)

set(scripting_SRC
    scripting/appinterface.cpp
    scripting/applet.cpp
    scripting/containment.cpp
    scripting/configgroup.cpp
    scripting/i18n.cpp
    scripting/layouttemplatepackagestructure.cpp
    scripting/rect.cpp
    scripting/scriptengine.cpp
    scripting/widget.cpp
)

set(plasmagenericshell_LIB_SRCS
    backgrounddialog.cpp
    mouseinputbutton.cpp
    mouseplugins.cpp
    mousepluginwidget.cpp
    panelshadows.cpp
    toolbutton.cpp
    wallpaperpreview.cpp
    ${scripting_SRC}
    ${appletbrowserdialog_SRCS}
)

kde4_add_ui_files(plasmagenericshell_LIB_SRCS BackgroundDialog.ui MousePlugins.ui)

add_library(plasmagenericshell SHARED ${plasmagenericshell_LIB_SRCS})

target_link_libraries(plasmagenericshell KF5::Plasma Qt5::Script Qt5::Declarative Qt5::UiTools kworkspace)
if(UNIX AND HAVE_X11)
    target_link_libraries(plasmagenericshell ${X11_LIBRARIES})
endif()

set_target_properties(plasmagenericshell PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION} )

install(DIRECTORY widgetsexplorer/package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.desktop.widgetexplorer
        PATTERN "Messages.sh" EXCLUDE
)

install(TARGETS plasmagenericshell EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES plasma-layout-template.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

