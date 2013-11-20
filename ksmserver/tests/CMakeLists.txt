include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

set(testsh_SRCS
    test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shutdowndlg.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../logouteffect.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../curtaineffect.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../fadeeffect.cpp)
)

add_executable(testsh TEST ${testsh_SRCS})

target_link_libraries(testsh
    KF5::Plasma
    kworkspace
    ${KDE4_KDEUI_LIBS}
    ${KDE4_SOLID_LIBRARY}
    ${X11_X11_LIB}
    ${KDECLARATIVE_LIBRARIES}
    Qt5::Declarative
)
