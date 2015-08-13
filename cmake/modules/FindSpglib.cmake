# - Try to find libspg
# Once done this will define
#  SPGLIB_FOUND - System has spglib
#  SPGLIB_INCLUDE_DIRS - The spglib include directories
#  SPGLIB_LIBRARIES - The libraries needed to use spglib

find_path(SPGLIB_INCLUDE_DIR spglib/spglib.h
  PATHS
  ${INCLUDE_INSTALL_DIR}
  PATH_SUFFIXES spglib )

find_library(SPGLIB_LIBRARY NAMES symspg
  PATHS
  ${LIBRARY_INSTALL_DIR} )


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set SPGLIB_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(SPGLIB  DEFAULT_MSG
                                  SPGLIB_LIBRARY SPGLIB_INCLUDE_DIR)

mark_as_advanced(SPGLIB_INCLUDE_DIR SPGLIB_LIBRARY )
