# - Try to find the a valid boost+python combination
# Once done this will define
#
#  BOOST_PYTHON_FOUND - system has a valid boost+python combination
#  BOOST_PYTHON_INCLUDES - the include directory for boost+python
#  BOOST_PYTHON_LIBS - the needed libs for boost+python

# Copyright (c) 2006, Pino Toscano, <toscano.pino@tiscali.it>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(BOOST_PYTHON_INCLUDES AND BOOST_PYTHON_LIBS)
    # Already in cache, be silent
	set(BOOST_PYTHON_FIND_QUIETLY TRUE)
endif(BOOST_PYTHON_INCLUDES AND BOOST_PYTHON_LIBS)

# some needed includes
include(CheckCXXSourceCompiles)
include(CheckIncludeFileCXX)
include(CheckLibraryExists)

# reset vars
set(BOOST_PYTHON_INCLUDES)
set(BOOST_PYTHON_LIBS)

# handy arrays
set(PYTHON_VERSIONS "python;python2.5;python2.4;python2.3;python2.2")
set(PYTHON_INCLUDE_DIRS "/usr/include/$pyver;/usr/local/include/$pyver;/usr/$pyver/include;/usr/local/$pyver/include;$prefix/include/$pyver;$prefix/$pyver/include")
set(PYTHON_LIBRARY_DIRS "/usr/lib${LIB_SUFFIX};/usr/local/lib${LIB_SUFFIX};/usr/lib${LIB_SUFFIX}/$pyver/config")
set(PYTHON_LIBS "boost_python-gcc-mt;boost_python-mt;boost_python-gcc-mt-1_33;boost_python-gcc-mt-1_33_1;boost_python;boost_python-gcc-mt-1_32;boost_python")

# 1st: check for boost/shared_ptr.hpp
check_include_file_cxx(boost/shared_ptr.hpp HAVE_BOOST_SHARED_PTR_HPP)

if(HAVE_BOOST_SHARED_PTR_HPP)

  set(_found FALSE)

  # save the old flags and setting the new ones
  set(_save_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

  foreach(_pyver ${PYTHON_VERSIONS})
    if(NOT _found)
      foreach(_pydir ${PYTHON_INCLUDE_DIRS})

        if(NOT _found)
          string(REPLACE "$pyver" "${_pyver}" _pydir ${_pydir})
          string(REPLACE "$prefix" "${CMAKE_INSTALL_PREFIX}" _pydir ${_pydir})

          if(EXISTS ${_pydir})
          if(EXISTS ${_pydir}/Python.h)

            foreach(_pylibdir ${PYTHON_LIBRARY_DIRS})

              if(NOT _found)
                string(REPLACE "$pyver" ${_pyver} _pylibdir ${_pylibdir})

                foreach(_pblib ${PYTHON_LIBS})

                  if(NOT _found)

                    set(CMAKE_REQUIRED_FLAGS "-L${_pylibdir}")
                    set(CMAKE_REQUIRED_INCLUDES ${_pydir})
                    set(CMAKE_REQUIRED_LIBRARIES ${_pblib} ${_pyver})

                    check_cxx_source_compiles("
#include <boost/python.hpp>
const char* greet() { return \"Hello world!\"; }
BOOST_PYTHON_MODULE(hello) { boost::python::def(\"greet\", greet); }

int main() { return 0; }

// some vars, in case of the compilation fail...
// python include dir: ${_pydir}
// python lib dir: ${_pylibdir}
// boost python lib: ${_pblib}

" boost_python_${_pylibdir}_${_pydir}_${_pblib}_compile )

                    set(CMAKE_REQUIRED_FLAGS)
                    set(CMAKE_REQUIRED_INCLUDES)
                    set(CMAKE_REQUIRED_LIBRARIES)

                    if(boost_python_${_pylibdir}_${_pydir}_${_pblib}_compile)

                        set(_found TRUE)

                        set(BOOST_PYTHON_INCLUDES ${_pydir})
                        set(BOOST_PYTHON_LIBS "-l${_pyver} -L${_pylibdir} -l${_pblib}")

                    endif(boost_python_${_pylibdir}_${_pydir}_${_pblib}_compile)

                  endif(NOT _found)

                endforeach(_pblib ${PYTHON_LIBS})

              endif(NOT _found)

            endforeach(_pylibdir ${PYTHON_LIBRARY_DIRS})

          endif(EXISTS ${_pydir}/Python.h)
          endif(EXISTS ${_pydir})

        endif(NOT _found)

      endforeach(_pydir ${PYTHON_INCLUDE_DIRS})

    endif(NOT _found)

  endforeach(_pyver ${PYTHON_VERSIONS})

  set(CMAKE_CXX_FLAGS ${_save_CMAKE_CXX_FLAGS})

endif(HAVE_BOOST_SHARED_PTR_HPP)

if(BOOST_PYTHON_INCLUDES AND BOOST_PYTHON_LIBS)
  set(BOOST_PYTHON_FOUND TRUE)
endif(BOOST_PYTHON_INCLUDES AND BOOST_PYTHON_LIBS)

if(BOOST_PYTHON_FOUND)
  if(NOT BoostPython_FIND_QUIETLY)
    message(STATUS "Found Boost+Python: ${BOOST_PYTHON_LIBS}")
  endif(NOT BoostPython_FIND_QUIETLY)
  set(KIG_ENABLE_PYTHON_SCRIPTING 1)
else (BOOST_PYTHON_FOUND)
  if (BoostPython_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find Boost+Python")
  endif(BoostPython_FIND_REQUIRED)
  set(KIG_ENABLE_PYTHON_SCRIPTING 0)
endif(BOOST_PYTHON_FOUND)

mark_as_advanced(BOOST_PYTHON_INCLUDES BOOST_PYTHON_LIBS)
