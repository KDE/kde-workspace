set(plasmasvgviewer_SRCS
    svgviewer.cpp
    main.cpp
    plasmaview.cpp
)

kde4_add_executable(plasmasvgviewer ${plasmasvgviewer_SRCS})

target_link_libraries(plasmasvgviewer ${KDE4_PLASMA_LIBS} plasmagenericshell ${KDE4_KIO_LIBS})

install(TARGETS plasmasvgviewer ${INSTALL_TARGETS_DEFAULT_ARGS})
