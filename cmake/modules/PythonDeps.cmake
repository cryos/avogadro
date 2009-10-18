# Find All python dependencies
# This CMake file defines
#   ALL_PYTHON_FOUND, if false, do not try to use python.

if(Boost_PYTHON_FOUND AND PYTHONLIBS_FOUND AND NUMPY_FOUND)
  # In cache already
  set(ALL_PYTHON_FOUND TRUE CACHE BOOL "True if all Python dependencies satisfied.")

else(Boost_PYTHON_FOUND AND PYTHONLIBS_FOUND AND NUMPY_FOUND)

  message(STATUS "Searching for python dependencies...")

  # Boost Python
  message(STATUS "[1/5] Boost Python")
  # CMake's new FindBoost has an option to look for additional versions
  set(Boost_ADDITIONAL_VERSIONS "1.40" "1.40.0" "1.39" "1.39.0" "1.38" "1.38.0" "1.37" "1.37.0")
  find_package(Boost COMPONENTS python)
  if (Boost_PYTHON_FOUND)
    message(STATUS "Boost Python found...")
  else (Boost_PYTHON_FOUND)
    message(STATUS "Boost Python NOT found - Python support disabled.")
  #  message(STATUS "debian/ubuntu: install the libboost-python-dev package.")
    set(ALL_PYTHON_FOUND FALSE CACHE BOOL "True if all Python dependencies satisfied.")
    return()
  endif (Boost_PYTHON_FOUND)

  # Python Libraries
  message(STATUS "[2/5] Python Libraries")
  find_package(PythonLibs)
  if (NOT PYTHONLIBS_FOUND)
    message(STATUS "Python libraries NOT found - Python support disabled.")
  #  message(STATUS "debian/ubuntu: install the python-dev package. Correct?????")
    set(ALL_PYTHON_FOUND FALSE CACHE BOOL "True if all Python dependencies satisfied.")
    return()
  endif (NOT PYTHONLIBS_FOUND)

  # Python Interpreter
  message(STATUS "[3/5] Python Interpreter")
  find_package(PythonInterp)
  if (NOT PYTHONINTERP_FOUND)
    message(STATUS "Python interpreter NOT found - Python support disabled.")
  #  message(STATUS "debian/ubuntu: install the python package.")
    set(ALL_PYTHON_FOUND FALSE CACHE BOOL "True if all Python dependencies satisfied.")
    return()
  endif (NOT PYTHONINTERP_FOUND)

  # Numpy
  message(STATUS "[4/5] Numpy Module")
  find_package(Numpy)
  if (NOT NUMPY_FOUND)
    message(STATUS "Numpy NOT found - Python support disabled.")
  #  message(STATUS "debian/ubuntu: install the python-numpy package.")
    set(ALL_PYTHON_FOUND FALSE CACHE BOOL "True if all Python dependencies satisfied.")
    return()
  endif (NOT NUMPY_FOUND)

  # SIP
  message(STATUS "[5/5] SIP Module")
  if (EXISTS ${PYTHON_INCLUDE_PATH}/sip.h)
    message(STATUS "Found sip.h header...")
    execute_process(COMMAND sip -V OUTPUT_VARIABLE SIP_VERSION)
    if (${SIP_VERSION} MATCHES "4.8")
      message(STATUS "using sip version 4.8...")
      add_definitions( -DSIP_4_8 )
    endif (${SIP_VERSION} MATCHES "4.8")
  else (EXISTS ${PYTHON_INCLUDE_PATH}/sip.h)
    message(STATUS "sip.h header NOT found - Python support disabled")
  #  message(STATUS "debian/ubuntu: install the python-sip4-dev package.")
    set(ALL_PYTHON_FOUND FALSE CACHE BOOL "True if all Python dependencies satisfied.")
    return()
  endif (EXISTS ${PYTHON_INCLUDE_PATH}/sip.h)

  # Everything was found, define ENABLE_PYTHON. This can be used in code:
  #
  # #ifdef ENABLE_PYTHON
  # ...
  # #endif
  add_definitions( -DENABLE_PYTHON )
  # to be used in subdir CMakeLists.txt files
  set(ALL_PYTHON_FOUND TRUE CACHE BOOL "True if all Python dependencies satisfied.")

endif(Boost_PYTHON_FOUND AND PYTHONLIBS_FOUND AND NUMPY_FOUND)

mark_as_advanced(ALL_PYTHON_FOUND)
