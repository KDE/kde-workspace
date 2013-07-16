project(plasma-wallpaper-image)

set(image_SRCS
    image.cpp
    imageplugin.cpp
    wallpaperpackage.cpp
    backgroundlistmodel.cpp
)


add_library(plasma_wallpaper_imageplugin SHARED ${image_SRCS})
target_link_libraries(plasma_wallpaper_imageplugin
	#         plasma
	 ${Plasma_LIBRARIES}
         ${Qt5Quick_LIBRARIES}
         ${Qt5Qml_LIBRARIES}
         ${KDE4_KIO_LIBS}
         ${KDE4_KDEUI_LIBS}
         ${KDE4_KFILE_LIBS}
         ${KDE4_KNEWSTUFF3_LIBS}
         ${KDE4_THREADWEAVER_LIBRARY}
         ${KDE4Support_LIBRARIES})

install(TARGETS plasma_wallpaper_imageplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/wallpapers/image)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/wallpapers/image)

install(FILES wallpaper.knsrc DESTINATION ${CONFIG_INSTALL_DIR})

installPackage(package org.kde.image plasma/wallpapers wallpaper)
