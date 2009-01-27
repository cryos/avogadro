# - Find All python stuff
# This module defines
#  ALL_PYTHON_FOUND, If false, do not try to use python.

message(STATUS "Searching for python dependencies...")

#
# Boost Python
#
message(STATUS "[1/5] Boost Python")
# Cmake's new FindBoost has an option to look for additional versions
# e.g., there's now a 1.37.0 (and thus 1.37) release.
set(Boost_ADDITIONAL_VERSIONS "1.37" "1.37.0")
find_package(Boost COMPONENTS python)
if (Boost_PYTHON_FOUND)
  message(STATUS "Boost Python found...")
else (Boost_PYTHON_FOUND)
  message(STATUS "Boost Python NOT found - Python support disabled.")
  message(STATUS "debian/ubuntu: install the libboost-python-dev package.")
  return()
endif (Boost_PYTHON_FOUND)
 
# 
# Python Libraries
# 
message(STATUS "[2/5] Python Libraries")
find_package(PythonLibs)
if (NOT PYTHONLIBS_FOUND)
  message(STATUS "Python Libraries NOT found - Python support disabled.")
  message(STATUS "debian/ubuntu: install the python-dev package. Correct?????")
  return()
endif (NOT PYTHONLIBS_FOUND)

# 
# Python Interpreter
#
message(STATUS "[3/5] Python Interpreter")
find_package(PythonInterp)
if (NOT PYTHONINTERP_FOUND)
  message(STATUS "Python Interpreter NOT found - Python support disabled.")
  message(STATUS "debian/ubuntu: install the python package.")
  return()
endif (NOT PYTHONINTERP_FOUND)
 
# 
# Numpy
#  
message(STATUS "[4/5] Numpy module")
find_package(Numpy)
if (NOT NUMPY_FOUND)
  message(STATUS "Numpy NOT found - Python support disabled.")
  message(STATUS "debian/ubuntu: install the python-numpy package.")
  return()
endif (NOT NUMPY_FOUND)
 
#  
# SIP 
#
message(STATUS "[5/5] SIP module")
if (EXISTS ${PYTHON_INCLUDE_PATH}/sip.h)
  message(STATUS "Found sip.h header...")
else (EXISTS ${PYTHON_INCLUDE_PATH}/sip.h)
  message(STATUS "sip.h header NOT found - Python support disabled")
  message(STATUS "debian/ubuntu: install the python-sip4-dev package.")
  return()
endif (EXISTS ${PYTHON_INCLUDE_PATH}/sip.h)

# everything is found, define ENABLE_PYTHON. This can be used in code:
#
# #ifdef ENABLE_PYTHON
# ...
# #endif
add_definitions( -DENABLE_PYTHON )
# to be used in subdir CMakeLists.txt files
set(ALL_PYTHON_FOUND True)
