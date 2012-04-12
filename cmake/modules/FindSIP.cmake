# - Find sip
# Find the python sip includes making sure it is loadable from python
# This module defines
#  SIP_INCLUDE_DIR, where to find sip.h, etc.
#  SIP_FOUND, If false, do not try to use numpy headers.

# be certain sip is available from python.  this part might be unnecessary.
execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
  "import sip; print sip.SIP_VERSION"
  OUTPUT_VARIABLE SIP_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)

if(SIP_VERSION)
  set (SIP_PYTHON_FOUND TRUE)
else()
  message(STATUS "SIP module not available in Python." )
  set (SIP_PYTHON_FOUND FALSE)
endif()

if(SIP_PYTHON_FOUND)
  # search for sip.h
  if(SIP_INCLUDE_DIR)
    set(SIP_FOUND TRUE)
  else()
    find_path(SIP_INCLUDE_DIR NAMES sip.h
     PATHS
     PATH_SUFFIXES SIP
    )
  endif()
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(SIP DEFAULT_MSG SIP_INCLUDE_DIR )
else()
  # give up
  set(SIP_FOUND FALSE)
endif()

mark_as_advanced(SIP_INCLUDE_DIR)
