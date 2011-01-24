include_directories(
  ${libavogadro_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
  ${OPENBABEL2_INCLUDE_DIR}
  ${BOOST_PYTHON_INCLUDES}
  ${PYTHON_INCLUDE_PATH}
  ${NUMPY_INCLUDE_DIR}
)

# use all cpp files in this directory
file(GLOB wrapper_SRCS "*.cpp")

qt4_automoc(moleculelist.cpp)

add_library(python-module MODULE ${wrapper_SRCS})
set_target_properties(python-module PROPERTIES OUTPUT_NAME Avogadro)
set_target_properties(python-module PROPERTIES PREFIX "")
if (WIN32)
  set_target_properties(python-module PROPERTIES SUFFIX ".pyd")
  target_link_libraries(python-module avogadro ${PYTHON_LIBRARIES})
else()
  target_link_libraries(python-module stdc++ avogadro
    ${QT_LIBRARIES} ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})
endif()

# Let's try to use this instead:
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c
    "from sys import stdout; from distutils import sysconfig; stdout.write(sysconfig.get_python_lib(1))"
    OUTPUT_VARIABLE PYTHON_LIB_PATH)

message(STATUS "Python site-packages directory: ${PYTHON_LIB_PATH}")

option(INSTALL_PYTHON_MODULE 
  "Install the avogadro python module for the system wide python installation."
  ON)
if (INSTALL_PYTHON_MODULE)
  install(TARGETS python-module DESTINATION ${PYTHON_LIB_PATH})
endif()

