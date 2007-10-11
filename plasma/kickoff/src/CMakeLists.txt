# Kickoff Shared
set( Kickoff_SRCS
       core/applicationmodel.cpp
       core/favoritesmodel.cpp
       core/leavemodel.cpp
       core/models.cpp
       core/recentapplications.cpp
       core/recentlyusedmodel.cpp
       core/searchmodel.cpp
       core/systemmodel.cpp
       core/urlitemlauncher.cpp
       core/itemhandlers.cpp
       ui/itemdelegate.cpp
       ui/contextmenufactory.cpp
       ui/flipscrollview.cpp
       ui/launcher.cpp
       ui/searchbar.cpp
       ui/tabbar.cpp
       ui/urlitemview.cpp  )

set ( Kickoff_LIBS
        ${KDE4_KIO_LIBS} 
        ${KDE4_SOLID_LIBS}
        kworkspace 
        strigiqtdbusclient )

# Standalone Test Application
set ( Application_SRCS
        ${Kickoff_SRCS}
        main.cpp )

kde4_add_executable(kickoff ${Application_SRCS})
target_link_libraries(kickoff ${Kickoff_LIBS})

install(TARGETS kickoff DESTINATION ${BIN_INSTALL_DIR})

# Plasma Applet
set ( Applet_SRCS
        ${Kickoff_SRCS}
        plasma-applet/applet.cpp )

kde4_add_plugin(kickoff_plasma_applet_launcher ${Applet_SRCS})
target_link_libraries(kickoff_plasma_applet_launcher ${Kickoff_LIBS} plasma)
install(TARGETS kickoff_plasma_applet_launcher DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet/plasma-launcher-applet.desktop DESTINATION ${SERVICES_INSTALL_DIR})

