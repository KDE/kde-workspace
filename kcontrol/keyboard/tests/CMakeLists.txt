set ( text_paint_LIB ${KDE4_PLASMA_LIBS} )

MACRO(KEYBOARD_DAEMON_UNIT_TESTS _testname)
               kde4_add_unit_test(${_testname}_test TESTNAME kcm-keyboard-${_testname} NOGUI ${_testname}_test.cpp ../${_testname}.cpp ../x11_helper.cpp)
               target_link_libraries(${_testname}_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB} ${X11_LIBRARIES})
ENDMACRO(KEYBOARD_DAEMON_UNIT_TESTS)

MACRO(KEYBOARD_DAEMON_UNIT_TESTS2 _testname _src1 _src2 _src3)
               kde4_add_unit_test(${_testname}_test TESTNAME kcm-keyboard-${_testname} NOGUI ${_testname}_test.cpp ../${_testname}.cpp ../${_src1} ../${_src2} ../${_src3})
               target_link_libraries(${_testname}_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB})
ENDMACRO(KEYBOARD_DAEMON_UNIT_TESTS2)


KEYBOARD_DAEMON_UNIT_TESTS(xkb_rules)
KEYBOARD_DAEMON_UNIT_TESTS(iso_codes)

if(Boost_FOUND)
    include_directories("../preview")

    set (CMAKE_CXX_FLAGS "-fexceptions")
    set (CMAKE_CXX_FLAGS "-fpermissive")

    ADD_DEFINITIONS(-DNEW_GEOMETRY=1)
    include_directories(..)

    kde4_add_unit_test(geometry_parser_test TESTNAME geometry_parser NOGUI ../preview/geometry_parser.cpp ../preview/geometry_components.cpp geometry_parser_test.cpp ../x11_helper.cpp ../preview/model_to_geometry.cpp)
    
    kde4_add_unit_test(symbol_parser_test TESTNAME symbol_parser NOGUI ../preview/symbol_parser.cpp ../preview/keyboardlayout.cpp symbol_parser_test.cpp ../x11_helper.cpp ../preview/keyaliases.cpp)
    
    kde4_add_unit_test(base_parser_test TESTNAME base_parser NOGUI ../preview/model_to_geometry.cpp ../x11_helper.cpp base_file_parser_test.cpp )
    
    target_link_libraries(geometry_parser_test ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${X11_LIBRARIES})
    target_link_libraries(symbol_parser_test ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${X11_LIBRARIES})
    target_link_libraries(base_parser_test ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${X11_LIBRARIES})
    
endif(Boost_FOUND)

kde4_add_unit_test(flags_test TESTNAME kcm-keyboard-flags_test NOGUI flags_test.cpp ../flags.cpp ../x11_helper.cpp ../keyboard_config.cpp ../xkb_rules.cpp)
target_link_libraries(flags_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB} ${X11_LIBRARIES} ${text_paint_LIB})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.1.1.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.1.1.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.bad.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.bad.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.xml COPYONLY)

if( X11_XTest_FOUND )
   kde4_add_unit_test(keyboard_daemon_test TESTNAME kcm-keyboard-keyboard_daemon_test NOGUI keyboard_daemon_test.cpp ../keyboard_daemon.cpp ../layout_memory.cpp ../layout_memory_persister.cpp ../flags.cpp ../x11_helper.cpp ../xkb_helper.cpp ../xinput_helper.cpp ../layout_tray_icon.cpp ../keyboard_config.cpp ../xkb_rules.cpp ../keyboard_hardware.cpp ../bindings.cpp ../layouts_menu.cpp ../numlockx.c)
   target_link_libraries(keyboard_daemon_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB} ${X11_XTest_LIB} ${X11_LIBRARIES} ${text_paint_LIB})
   if(XiGetDevicePresence_FOUND)
      target_link_libraries(keyboard_daemon_test ${X11_Xinput_LIB})
   endif(XiGetDevicePresence_FOUND)
endif( X11_XTest_FOUND )
