include_directories(
  ${libavogadro_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${OPENBABEL2_INCLUDE_DIR}
  ${BOOST_PYTHON_INCLUDES}
  ${PYTHON_INCLUDE_PATH}
  ${NUMPY_INCLUDE_DIR}
)

# use all cpp files in this directory
FILE(GLOB wrapper_SRCS "*.cpp")

qt4_automoc(moleculelist.cpp)

ADD_LIBRARY(python-module MODULE ${wrapper_SRCS})
SET_TARGET_PROPERTIES(python-module PROPERTIES OUTPUT_NAME Avogadro)
SET_TARGET_PROPERTIES(python-module PROPERTIES PREFIX "")
if (WIN32)
  SET_TARGET_PROPERTIES(python-module PROPERTIES SUFFIX ".pyd")
  TARGET_LINK_LIBRARIES(python-module avogadro ${PYTHON_LIBRARIES})
else (WIN32)
  TARGET_LINK_LIBRARIES(python-module stdc++ avogadro periodictable
    ${QT_LIBRARIES} ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})
endif (WIN32)

# Let's try to use this instead:
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c
    "from sys import stdout; from distutils import sysconfig; stdout.write(sysconfig.get_python_lib())"
    OUTPUT_VARIABLE PYTHON_LIB_PATH)

message(STATUS "Python site-packages directory: ${PYTHON_LIB_PATH}")

option(INSTALL_PYTHON_MODULE "Install the avogadro python module for the system wide python installation." ON)
if (INSTALL_PYTHON_MODULE)
INSTALL(TARGETS python-module DESTINATION ${PYTHON_LIB_PATH})
endif (INSTALL_PYTHON_MODULE)

