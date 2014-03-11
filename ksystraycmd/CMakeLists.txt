set(ksystraycmd_SRCS ksystraycmd.cpp main.cpp )


add_executable(ksystraycmd ${ksystraycmd_SRCS})

target_link_libraries(ksystraycmd KF5::KDE4Support Qt5::X11Extras)
if (HAVE_X11)
  target_link_libraries(ksystraycmd ${X11_X11_LIB})
endif ()

install(TARGETS ksystraycmd ${INSTALL_TARGETS_DEFAULT_ARGS})
