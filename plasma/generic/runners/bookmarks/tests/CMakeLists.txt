enable_testing()
set( testChromeBookmarks_SRCS testchromebookmarks.cpp 
  ../browsers/chrome.cpp
  ../browser.cpp
  ../browsers/firefox.cpp
  ../browsers/kdebrowser.cpp
  ../browsers/opera.cpp
  ../browsers/chromebookmarksfinder.cpp
)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/.. )
kde4_add_unit_test( testChromeBookmarks TESTNAME TestChromeBookmarks ${testChromeBookmarks_SRCS} )

target_link_libraries( testChromeBookmarks ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} )
