include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcheckpass
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kdm
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace
)

########### next target ###############
set(kscreenunlocker_SRCS
   unlockapp.cpp
   unlocker.cpp
   main.cpp )

kde4_add_kcfg_files(kscreenunlocker_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../kcfg/kscreensaversettings.kcfgc)

kde4_add_executable(kscreenunlocker ${kscreenunlocker_SRCS})

target_link_libraries(kscreenunlocker kephal kworkspace ${KDE4_KDEUI_LIBS} ${QT_QTDECLARATIVE_LIBRARY} kdeclarative)

install(TARGETS kscreenunlocker DESTINATION ${LIBEXEC_INSTALL_DIR})
install(FILES
    qml/Greeter.qml
    qml/lockscreen.qml
    qml/SessionSwitching.qml
    DESTINATION ${DATA_INSTALL_DIR}/ksld)
