FILE(GLOB wrapper_SRCS "*.cpp")
ADD_LIBRARY(python-module SHARED ${wrapper_SRCS})
SET_TARGET_PROPERTIES(python-module PROPERTIES OUTPUT_NAME Avogadro)
SET_TARGET_PROPERTIES(python-module PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(python-module stdc++ avogadro-lib)

