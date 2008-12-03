include_directories(
  ${libavogadro_SOURCE_DIR}/include
  ${OPENBABEL2_INCLUDE_DIR}
  ${BOOST_PYTHON_INCLUDES}
  ${PYTHON_INCLUDE_PATH}
  ${NUMPY_INCLUDE_DIR}
)

FILE(GLOB wrapper_SRCS "*.cpp")
ADD_LIBRARY(python-module SHARED ${wrapper_SRCS})
SET_TARGET_PROPERTIES(python-module PROPERTIES OUTPUT_NAME Avogadro)
SET_TARGET_PROPERTIES(python-module PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(python-module stdc++ avogadro-lib)

