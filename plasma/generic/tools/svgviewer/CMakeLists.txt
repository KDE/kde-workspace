set(plasmasvgviewer_SRCS
    svgviewer.cpp
    main.cpp
)

kde4_add_ui_files(plasmasvgviewer.ui svgviewer.ui)
kde4_add_executable(plasmasvgviewer ${plasmasvgviewer_SRCS})

target_link_libraries(plasmasvgviewer ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasmasvgviewer ${INSTALL_TARGETS_DEFAULT_ARGS})
