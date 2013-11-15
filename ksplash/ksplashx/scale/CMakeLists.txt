########### next target ###############

set( ksplashx_scale_SRCS
    main.cpp )

add_executable( ksplashx_scale ${ksplashx_scale_SRCS})
target_link_libraries( ksplashx_scale  Qt5::Gui)
install( TARGETS ksplashx_scale ${INSTALL_TARGETS_DEFAULT_ARGS} )
