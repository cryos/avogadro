# Find All python dependencies
# This CMake file defines
#   ALL_PYTHON_FOUND, if false, do not try to use python.

if(Boost_PYTHON_FOUND AND PYTHONLIBS_FOUND AND NUMPY_FOUND)
  # In cache already
  set(ALL_PYTHON_FOUND TRUE)

else()
  message(STATUS "Searching for python dependencies...")

  # Boost Python
  message(STATUS "[1/5] Boost Python")
  # CMake's new FindBoost has an option to look for additional versions
  set(Boost_ADDITIONAL_VERSIONS "1.45" "1.44" "1.43" "1.42" "1.41" "1.40"
    "1.40.0" "1.39" "1.39.0" "1.38" "1.38.0" "1.37" "1.37.0")
  find_package(Boost COMPONENTS python)
  if (Boost_PYTHON_FOUND)
    message(STATUS "Boost Python found...")
  else()
    message(STATUS "Boost Python NOT found - Python support disabled.")
  #  message(STATUS "debian/ubuntu: install the libboost-python-dev package.")
    set(ALL_PYTHON_FOUND FALSE)
    return()
  endif()

  # Python Libraries
  message(STATUS "[2/5] Python Libraries")
  find_package(PythonLibs)
  if (NOT PYTHONLIBS_FOUND)
    message(STATUS "Python libraries NOT found - Python support disabled.")
  #  message(STATUS "debian/ubuntu: install the python-dev package. Correct?????")
    set(ALL_PYTHON_FOUND FALSE)
    return()
  endif()

  # Python Interpreter
  message(STATUS "[3/5] Python Interpreter")
  find_package(PythonInterp)
  if (NOT PYTHONINTERP_FOUND)
    message(STATUS "Python interpreter NOT found - Python support disabled.")
  #  message(STATUS "debian/ubuntu: install the python package.")
    set(ALL_PYTHON_FOUND FALSE)
    return()
  endif()

  # Numpy
  message(STATUS "[4/5] Numpy Module")
  find_package(Numpy)
  if (NOT NUMPY_FOUND)
    message(STATUS "Numpy NOT found - Python support disabled.")
  #  message(STATUS "debian/ubuntu: install the python-numpy package.")
    set(ALL_PYTHON_FOUND FALSE)
    return()
  endif()

  # SIP
  message(STATUS "[5/5] SIP Module")
  if (EXISTS ${PYTHON_INCLUDE_PATH}/sip.h)
    message(STATUS "Found sip.h header...")
  else ()
    message(STATUS "sip.h header NOT found - Python support disabled")
  #  message(STATUS "debian/ubuntu: install the python-sip4-dev package.")
    set(ALL_PYTHON_FOUND FALSE)
    return()
  endif()

  # Everything was found, define ENABLE_PYTHON. This can be used in code:
  #
  # #ifdef ENABLE_PYTHON
  # ...
  # #endif
  add_definitions( -DENABLE_PYTHON )
  # to be used in subdir CMakeLists.txt files
  set(ALL_PYTHON_FOUND TRUE)

endif()
