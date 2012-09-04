########### next target ###############
set(EXTERNAL_LIBS ${KDE4_KIO_LIBS} ${QT_QTSCRIPT_LIBRARY} ${KDE4_PLASMA_LIBS} ${QT_QTSQL_LIBRARY})
if(QJSON_FOUND)
    add_definitions(-DHAVE_QJSON=1)
    set(EXTERNAL_LIBS ${EXTERNAL_LIBS} ${QJSON_LIBRARIES} )
    include_directories(${QJSON_INCLUDE_DIR})
endif(QJSON_FOUND)

set(krunner_bookmarksrunner_SRCS
    browserfactory.cpp
    bookmarkmatch.cpp
    faviconfromblob.cpp
    favicon.cpp
    fetchsqlite.cpp
    browsers/opera.cpp
    bookmarksrunner.cpp
    browsers/kdebrowser.cpp
    browsers/firefox.cpp
)

if(QJSON_FOUND)
set(krunner_bookmarksrunner_SRCS
    ${krunner_bookmarksrunner_SRCS}
    browsers/chromefindprofile.cpp
    browsers/chrome.cpp
)
endif(QJSON_FOUND)


kde4_add_plugin(krunner_bookmarksrunner ${krunner_bookmarksrunner_SRCS})
target_link_libraries(krunner_bookmarksrunner ${EXTERNAL_LIBS})

install(TARGETS krunner_bookmarksrunner DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install(FILES plasma-runner-bookmarks.desktop DESTINATION ${SERVICES_INSTALL_DIR})

# Currently tests include only chrome, so no need to get include them if json is not found
if(QJSON_FOUND)
    add_subdirectory(tests)
endif(QJSON_FOUND)
