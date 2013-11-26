include_directories(
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kdm
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/plasmagenericshell
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/screenlocker/greeter/
    ${CMAKE_BINARY_DIR}/ksmserver/screenlocker/greeter/
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcheckpass
)

set(plasma-overlay_SRCS
    backgrounddialog.cpp
    main.cpp
    plasmaapp.cpp
    savercorona.cpp
    saverview.cpp
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/screenlocker/greeter/greeter.cpp
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/screenlocker/greeter/sessions.cpp
)

kde4_add_ui_files(plasma-overlay_SRCS BackgroundDialog.ui)

kconfig_add_kcfg_files(plasma-overlay_SRCS ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/screenlocker/kcfg/kscreensaversettings.kcfgc)

set(plasmaapp_dbusXML org.kde.plasma-overlay.App.xml)
#qt5_generate_dbus_interface(plasmaapp.h ${plasmaapp_dbusXML} OPTIONS -S -M)
qt5_add_dbus_adaptor(plasma-overlay_SRCS ${plasmaapp_dbusXML} plasmaapp.h PlasmaApp)

add_executable(plasma-overlay ${plasma-overlay_SRCS})

target_link_libraries(plasma-overlay KF5::Plasma kworkspace  KIOCore KF5::KIOFileWidgets
                                     ${X11_X11_LIB} plasmagenericshell Qt5::Declarative kdeclarative)
if(X11_Xrender_FOUND)
  target_link_libraries(plasma-overlay ${X11_Xrender_LIB})
endif()
set_target_properties(plasma-overlay PROPERTIES OUTPUT_NAME plasma-overlay)

install(TARGETS plasma-overlay ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES plasma-overlayrc DESTINATION ${CONFIG_INSTALL_DIR})
