if(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
	#add_subdirectory(animators)
endif()

add_subdirectory(containmentactions)

add_subdirectory(applets)
#add_subdirectory(runners)
#add_subdirectory(scriptengines)
#add_subdirectory(shells)

add_subdirectory(dataengines)
add_subdirectory(wallpapers)

