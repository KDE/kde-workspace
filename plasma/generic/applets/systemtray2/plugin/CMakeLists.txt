set(systemtrayplugin_SRCS
    systemtrayplugin.cpp
    systemtraymanager.cpp
    )

add_library(systemtrayplugin SHARED ${systemtrayplugin_SRCS})

target_link_libraries(systemtrayplugin
    ${Qt5Quick_LIBRARIES}
    ${Qt5Qml_LIBRARIES}
    ${KCore_LIBRARIES}
    ${KDECLARATIVE_LIBRARIES}
    ${Plasma_LIBRARIES}
    KF5::KWindowSystem
    )

#install(TARGETS systemtrayplugin DESTINATION ${QML_INSTALL_DIR}/private/org/kde/plasma/systemtray)
#install(FILES qmldir ToolTip.qml DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/core)

