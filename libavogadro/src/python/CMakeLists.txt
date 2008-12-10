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
  add_definitions( -DENABLE_PYTHON_SIP )
endif (EXISTS ${PYTHON_INCLUDE_PATH}/sip.h)

ADD_LIBRARY(python-module MODULE ${wrapper_SRCS})
SET_TARGET_PROPERTIES(python-module PROPERTIES OUTPUT_NAME Avogadro)
SET_TARGET_PROPERTIES(python-module PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(python-module stdc++ avogadro-lib)

# Let's try to use this instead:
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c 
    "from sys import stdout; from distutils import sysconfig; stdout.write(sysconfig.get_python_lib())"
    OUTPUT_VARIABLE PYTHON_LIB_PATH)

message(STATUS "Python site-packages directory: ${PYTHON_LIB_PATH}")

INSTALL(TARGETS python-module DESTINATION ${PYTHON_LIB_PATH})

