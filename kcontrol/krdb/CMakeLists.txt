
add_subdirectory( themes ) 
add_subdirectory( ad ) 
add_subdirectory( pics ) 

install(FILES krdb_libpathwipe.upd DESTINATION ${DATA_INSTALL_DIR}/kconf_update)

kde4_add_executable(krdb_clearlibrarypath krdb_clearlibrarypath.cpp)
target_link_libraries(krdb_clearlibrarypath Qt5::Core ${KDE4Support_LIBRARIES})
install(TARGETS krdb_clearlibrarypath DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin)
