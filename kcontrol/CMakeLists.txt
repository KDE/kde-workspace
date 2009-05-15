add_subdirectory( colors )
add_subdirectory( krdb )
add_subdirectory( style )
add_subdirectory( standard_actions )
add_subdirectory( keys )

#TODO needs porting to what changed in KDE4 + please also rethink if it's really needed/useful to provide a duplicated nav-hierachy for colors, fonts, etc. - thx :)
#add_subdirectory( kthememanager )

add_subdirectory( hardware )

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KSMCARD_SUBDIR)")

