if(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
	#add_subdirectory(animators)
endif()

if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
	   add_subdirectory(containmentactions)
endif()

add_subdirectory(applets)
#add_subdirectory(runners)
#add_subdirectory(scriptengines)
#add_subdirectory(shells)

add_subdirectory(dataengines)
add_subdirectory(wallpapers)

