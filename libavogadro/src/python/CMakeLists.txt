include_directories(
  ${libavogadro_SOURCE_DIR}/include
  ${OPENBABEL2_INCLUDE_DIR}
  ${BOOST_PYTHON_INCLUDES}
  ${PYTHON_INCLUDE_PATH}
  ${NUMPY_INCLUDE_DIR}
)

# use all cpp files in this directory
FILE(GLOB wrapper_SRCS "*.cpp")

# make sure we have sip
if (EXISTS ${PYTHON_INCLUDE_PATH}/sip.h)
  message(STATUS "Found sip.h - Enabled PyQt support")
  set(PYTHON_SIP_FOUND TRUE)
endif (EXISTS ${PYTHON_INCLUDE_PATH}/sip.h)

# remove sip.cpp from the list if we don't have sip.h
if (NOT PYTHON_SIP_FOUND)
  list(REMOVE_ITEM wrapper_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/sip.cpp)
endif (NOT PYTHON_SIP_FOUND)

ADD_LIBRARY(python-module SHARED ${wrapper_SRCS})
SET_TARGET_PROPERTIES(python-module PROPERTIES OUTPUT_NAME Avogadro)
SET_TARGET_PROPERTIES(python-module PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(python-module stdc++ avogadro-lib)

#message("Looking for python site-packages directory...")
set(PYTHON_VERSIONS "python;python2.5;python2.4;python2.3;python2.2")
set(PYTHON_LIBRARY_DIRS "/usr/lib${LIB_SUFFIX};/usr/local/lib${LIB_SUFFIX};/usr/lib${LIB_SUFFIX}/$pyver")
set(_found FALSE)
foreach(_pyver ${PYTHON_VERSIONS})
  if (NOT _found)
    foreach (_pylibdir ${PYTHON_LIBRARY_DIRS})
      if (NOT _found)
        #message("before -> _pylibdir = " ${_pylibdir})
        string(REPLACE "$pyver" ${_pyver} _pylibdir ${_pylibdir})
        if (EXISTS ${_pylibdir}/site-packages)
          #message("after -> _pylibdir = " ${_pylibdir})
          set(_found TRUE)
          set(PYTHON_SITE_PACKAGES_DIR ${_pylibdir}/site-packages)
        endif (EXISTS ${_pylibdir}/site-packages)
      endif(NOT _found)
    endforeach(_pylibdir ${PYTHON_LIBRARY_DIRS})
  endif(NOT _found)
endforeach(_pyver ${PYTHON_VERSIONS})

message(STATUS "Python site-packages directory: ${PYTHON_SITE_PACKAGES_DIR}")

INSTALL(TARGETS python-module DESTINATION ${PYTHON_SITE_PACKAGES_DIR})

