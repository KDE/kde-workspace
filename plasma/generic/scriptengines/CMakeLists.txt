macro_optional_add_subdirectory(ruby)

if (PYTHONLIBRARY_FOUND AND NOT WIN32)
    macro_optional_add_subdirectory(python)
endif ()
