add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1223)
add_subdirectory(unlocker)

# for dbus_add_activation_service
include(PkgConfigGetVar)

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)

set(ksld_SRCS
   ksldapp.cpp
   lockwindow.cpp
   main.cpp)

kde4_add_kdeinit_executable(ksld ${ksld_SRCS})

target_link_libraries(kdeinit_ksld kworkspace ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES})

install(TARGETS kdeinit_ksld ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS ksld         ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES ksld.desktop  DESTINATION  ${AUTOSTART_INSTALL_DIR})
