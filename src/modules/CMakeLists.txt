include(compile_library)
set(link_libs "" PARENT_SCOPE)
COMPILE_LIBRARY(STATIC LIBS link_libs)
message(STATUS "after COMPILE_LIBRARY, link_libs=${link_libs}")
