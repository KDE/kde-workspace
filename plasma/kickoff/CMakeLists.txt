project(kickoff-kde4-rewrite)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include_directories(${KDE4_INCLUDES})

add_subdirectory(src)
