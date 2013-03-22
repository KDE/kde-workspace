macro_optional_add_subdirectory(ruby)

if(QT_QTWEBKIT_FOUND)
    macro_optional_add_subdirectory(webkit)
endif(QT_QTWEBKIT_FOUND)

if (PYTHONLIBRARY_FOUND AND NOT WIN32)
    macro_optional_add_subdirectory(python)
endif (PYTHONLIBRARY_FOUND AND NOT WIN32)
