if (0)
if(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
    add_subdirectory(animators)
endif(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)

if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   add_subdirectory(containmentactions)
endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")

add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)
add_subdirectory(scriptengines)
add_subdirectory(shells)
endif (0)

add_subdirectory(wallpapers)

