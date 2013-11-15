macro(KHOTKEYS_UNIT_TESTS)
       foreach(_testname ${ARGN})
               kde4_add_unit_test(${_testname} NOGUI ${_testname}.cpp)
               target_link_libraries(${_testname} khotkeysprivate  Qt5::Test)
       endforeach(_testname)
endmacro(KHOTKEYS_UNIT_TESTS)

khotkeys_unit_tests(
    kdedmodule_daemon_test )

