# - Find sip 
# Find the python sip includes making sure it is loadable from python
# This module defines
#  SIP_INCLUDE_DIR, where to find sip.h, etc.
#  SIP_FOUND, If false, do not try to use numpy headers.

# be certain sip is available from python.  this part might be unnecessary.
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c 
    "import sip; print sip.SIP_VERSION"
    OUTPUT_VARIABLE SIP_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
 
if (SIP_VERSION)
  set (SIP_PYTHON_FOUND TRUE)
else (SIP_VERSION)
  message(STATUS "SIP module not available in Python." )
  set (SIP_PYTHON_FOUND FALSE)
endif (SIP_VERSION)

if (SIP_PYTHON_FOUND) 
  # search for sip.h
  if (SIP_INCLUDE_DIR) 
    set (SIP_FOUND TRUE)
  else (SIP_INCLUDE_DIR) 
    find_path(SIP_INCLUDE_DIR NAMES sip.h
     PATHS
     PATH_SUFFIXES SIP
    )
  endif (SIP_INCLUDE_DIR) 
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(SIP DEFAULT_MSG SIP_INCLUDE_DIR )
else (SIP_PYTHON_FOUND) 
  # give up 
  set (SIP_FOUND FALSE)
endif (SIP_PYTHON_FOUND) 

MARK_AS_ADVANCED (SIP_INCLUDE_DIR)
