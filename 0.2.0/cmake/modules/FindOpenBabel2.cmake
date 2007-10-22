# - Try to find OpenBabel2
# Once done this will define
#
#  OPENBABEL2_FOUND - system has OpenBabel2
#  OPENBABEL2_INCLUDE_DIR - the OpenBabel2 include directory
#  OPENBABEL2_LIBRARIES - Link these to use OpenBabel2

# Copyright (c) 2006, Carsten Niehaus, <cniehaus@gmx.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (OPENBABEL2_INCLUDE_DIR AND OPENBABEL2_LIBRARIES)

  # in cache already
  set(OPENBABEL2_FOUND TRUE)

else (OPENBABEL2_INCLUDE_DIR AND OPENBABEL2_LIBRARIES)
if(NOT WIN32)
  include(UsePkgConfig)

  pkgconfig(openbabel-2.0 _obIncDir _obLinkDir _obLinkFlags _obCflags)

  # query pkg-config asking for a openbabel >= 2.1.0
  exec_program(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=2.1.0 openbabel-2.0 RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
  if(_return_VALUE STREQUAL "0")
	set(OPENBABEL_MINI_FOUND TRUE)
  endif(_return_VALUE STREQUAL "0")

  exec_program(${PKGCONFIG_EXECUTABLE} ARGS --variable=pkgincludedir openbabel-2.0 RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _obPkgIncDir )
  if (_obPkgIncDir)
    set(_obIncDir "${_obPkgIncDir}")
  endif (_obPkgIncDir)
else(NOT WIN32)
  set(OPENBABEL_MINI_FOUND TRUE)
endif(NOT WIN32)
  if(OPENBABEL_MINI_FOUND)

  find_path(OPENBABEL2_INCLUDE_DIR openbabel/obconversion.h
    ${_obIncDir}
    ${GNUWIN32_DIR}/include
    $ENV{OPENBABEL2_INCLUDE_DIR}
  )

  find_library(OPENBABEL2_LIBRARIES NAMES openbabel
    PATHS
    ${_obLinkDir}
    ${GNUWIN32_DIR}/lib
    $ENV{OPENBABEL2_LIBRARIES}
  )

  endif(OPENBABEL_MINI_FOUND)

  if(OPENBABEL2_INCLUDE_DIR AND OPENBABEL2_LIBRARIES AND OPENBABEL_MINI_FOUND)
    set(OPENBABEL2_FOUND TRUE)
  endif(OPENBABEL2_INCLUDE_DIR AND OPENBABEL2_LIBRARIES AND OPENBABEL_MINI_FOUND)

  if (OPENBABEL2_FOUND)
    if (NOT OpenBabel2_FIND_QUIETLY)
      message(STATUS "Found OpenBabel v2.1.0 or later: ${OPENBABEL2_LIBRARIES}")
    endif (NOT OpenBabel2_FIND_QUIETLY)
  else (OPENBABEL2_FOUND)
    if (OpenBabel2_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find OpenBabel 2.1.0 or later")
    endif (OpenBabel2_FIND_REQUIRED)
  endif (OPENBABEL2_FOUND)

  mark_as_advanced(OPENBABEL2_INCLUDE_DIR OPENBABEL2_LIBRARIES)

endif (OPENBABEL2_INCLUDE_DIR AND OPENBABEL2_LIBRARIES)

# Search for Open Babel2 executable
if(OPENBABEL2_EXECUTABLE)

  # in cache already
  set(OPENBABEL2_EXECUTABLE_FOUND TRUE)

else(OPENBABEL2_EXECUTABLE)
  find_program(OPENBABEL2_EXECUTABLE NAMES babel
    PATHS
    [HKEY_CURRENT_USER\\SOFTWARE\\OpenBabel\ 2.1.0]
    $ENV{OPENBABEL2_EXECUTABLE}
  )

  if(OPENBABEL2_EXECUTABLE)
    set(OPENBABEL2_EXECUTABLE_FOUND TRUE)
  endif(OPENBABEL2_EXECUTABLE)

  if(OPENBABEL2_EXECUTABLE_FOUND)
    message(STATUS "Found OpenBabel executable v2.1 or later: ${OPENBABEL2_EXECUTABLE}")
  endif(OPENBABEL2_EXECUTABLE_FOUND)

endif(OPENBABEL2_EXECUTABLE)

