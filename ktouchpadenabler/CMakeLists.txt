project(ktouchpadenabler)

find_package(KDE4 REQUIRED)
find_package(X11 REQUIRED)
include(KDE4Defaults)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QT4_INCLUDES} ${X11_INCLUDE_DIR})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

set(kded_ktouchpadenabler_SRCS
    ktouchpadenabler_daemon.cpp
)

kde4_add_kcfg_files(kded_ktouchpadenabler_SRCS settings.kcfgc )

kde4_add_plugin(kded_ktouchpadenabler
    ${kded_ktouchpadenabler_SRCS}
)

target_link_libraries(kded_ktouchpadenabler 
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${X11_X11_LIB}
    ${X11_Xinput_LIB}
)

install(TARGETS kded_ktouchpadenabler DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ktouchpadenabler.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
