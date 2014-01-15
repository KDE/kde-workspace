project(workspace-plasma)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
# add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
#
# remove_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1204)

add_subdirectory(desktop)
add_subdirectory(generic)
