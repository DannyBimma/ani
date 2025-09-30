# Third-party dependencies

# yyjson - fast JSON library
add_subdirectory(yyjson EXCLUDE_FROM_ALL)

# utf8proc - Unicode normalization (if enabled)
if(ANI_WITH_UTF8PROC)
  add_subdirectory(utf8proc EXCLUDE_FROM_ALL)
endif()

# Unity - test framework
add_subdirectory(unity EXCLUDE_FROM_ALL)