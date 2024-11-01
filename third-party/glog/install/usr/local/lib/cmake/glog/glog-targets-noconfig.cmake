#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "glog::glog" for configuration ""
set_property(TARGET glog::glog APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(glog::glog PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libglog.so.0.7.1"
  IMPORTED_SONAME_NOCONFIG "libglog.so.2"
  )

list(APPEND _cmake_import_check_targets glog::glog )
list(APPEND _cmake_import_check_files_for_glog::glog "${_IMPORT_PREFIX}/lib/libglog.so.0.7.1" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
