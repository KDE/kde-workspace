add_subdirectory(solid)
if(Nepomuk_FOUND)
  add_subdirectory(nepomukquery)
  add_subdirectory(nepomukqueryclient)
endif(Nepomuk_FOUND)

