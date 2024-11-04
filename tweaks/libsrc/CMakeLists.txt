# create library from source files

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

DCMTK_ADD_LIBRARY(tweaks
  tweaks.cc
)

DCMTK_TARGET_LINK_MODULES(tweaks ofstd oflog)
