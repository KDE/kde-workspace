########### next target ###############

set( prepareanim_SRCS
    prepareanim.cpp )

add_executable( prepareanim ${prepareanim_SRCS})
target_link_libraries( prepareanim  Qt5::Gui)

########### next target ###############

set( shade_SRCS
    shade.cpp )

add_executable( shade ${shade_SRCS})
target_link_libraries( shade  Qt5::Gui)
