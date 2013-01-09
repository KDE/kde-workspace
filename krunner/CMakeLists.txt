#KDE4_NO_ENABLE_FINAL(krunner)
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1207)

# for dbus_add_activation_service
include(PkgConfigGetVar)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard/processcore
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/plasmagenericshell
                    ${KDECLARATIVE_INCLUDE_DIR})

set(krunner_KCFG_SRCS kcfg/krunnersettings.kcfgc kcfg/klaunchsettings.kcfgc)
set(krunner_dbusAppXML dbus/org.kde.krunner.App.xml)


set(krunner_SRCS
   interfaceApi.cpp
   krunnerdialog.cpp
   krunnerapp.cpp
   main.cpp
   )

kde4_add_kcfg_files(krunner_SRCS ${krunner_KCFG_SRCS})

qt4_add_dbus_adaptor(krunner_SRCS ${krunner_dbusAppXML} krunnerapp.h KRunnerApp)
if(WIN32)
	set(BIN_PREFIX bin)
else(WIN32)
	set(BIN_PREFIX ${CMAKE_INSTALL_PREFIX}/bin)
endif(WIN32)

dbus_add_activation_service(dbus/org.kde.krunner.service.in)

if(NOT WIN32)
set(krunner_SRCS
    ${krunner_SRCS}
    ksystemactivitydialog.cpp
    startupid.cpp)

endif(NOT WIN32)

kde4_add_kdeinit_executable(krunner ${krunner_SRCS})

target_link_libraries(kdeinit_krunner kworkspace  plasmagenericshell ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBRARIES} ${KDE4_THREADWEAVER_LIBRARIES} ${KDE4_KDEUI_LIBS}  ${KDE4_KCMUTILS_LIBS} ${QT_QTDECLARATIVE_LIBRARY} ${KDECLARATIVE_LIBRARIES})

if(NOT WIN32)
  target_link_libraries(kdeinit_krunner processui  ${X11_Xrender_LIB} ${X11_LIBRARIES})
endif(NOT WIN32)

if(X11_Xss_LIB)
  target_link_libraries(kdeinit_krunner ${X11_Xss_LIB})
endif(X11_Xss_LIB)

if(X11_Xcursor_FOUND)
  target_link_libraries(kdeinit_krunner ${X11_Xcursor_LIB})
endif(X11_Xcursor_FOUND)

install(TARGETS kdeinit_krunner ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS krunner         ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES krunner.desktop  DESTINATION  ${AUTOSTART_INSTALL_DIR})
install(FILES kcfg/klaunch.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES ${krunner_dbusAppXML} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

add_subdirectory(interfaces)
