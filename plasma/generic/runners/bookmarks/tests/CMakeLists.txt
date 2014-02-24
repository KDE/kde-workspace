set( testChromeBookmarks_SRCS testchromebookmarks.cpp 
  ../browsers/chrome.cpp
  ../faviconfromblob.cpp
  ../browsers/chromefindprofile.cpp
  ../bookmarkmatch.cpp
  ../favicon.cpp
  ../fetchsqlite.cpp
)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/.. )
#kde4_add_unit_test( testChromeBookmarks TESTNAME plasma-runner-bookmarks-TestChromeBookmarks ${testChromeBookmarks_SRCS} )
add_test( testChromeBookmarks TESTNAME plasma-runner-bookmarks-TestChromeBookmarks ${testChromeBookmarks_SRCS} )

target_link_libraries( testChromeBookmarks  Qt5::Test Qt5::Sql KF5::Plasma KIOCore qjson)

file(COPY chrome-config-home DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
