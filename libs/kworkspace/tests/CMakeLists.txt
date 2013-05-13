set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ )

# Process unit test
kde4_add_unit_test(sesssiontest TESTNAME kdisplaymanager-sessiontest sessiontest.cpp)
target_link_libraries(sesssiontest kworkspace ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})
