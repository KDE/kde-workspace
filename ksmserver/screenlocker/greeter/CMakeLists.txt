include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcheckpass
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kdm
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace
)

set(kscreenlocker_greet_SRCS
   greeter.cpp
   sessions.cpp
   greeterapp.cpp
   main.cpp )

kde4_add_kcfg_files(kscreenlocker_greet_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../kcfg/kscreensaversettings.kcfgc)

kde4_add_executable(kscreenlocker_greet ${kscreenlocker_greet_SRCS})

target_link_libraries(kscreenlocker_greet kephal kworkspace ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTDECLARATIVE_LIBRARY} ${X11_LIBRARIES} ${KDE4_PLASMA_LIBS} ${KDE4_PLASMA_LIBS} ${KDECLARATIVE_LIBRARIES})

install(TARGETS kscreenlocker_greet DESTINATION ${LIBEXEC_INSTALL_DIR})

install(DIRECTORY themes/org.kde.passworddialog DESTINATION ${DATA_INSTALL_DIR}/ksmserver/screenlocker)