ADD_LIBRARY(python-module SHARED wrapper.cpp)
SET_TARGET_PROPERTIES(python-module PROPERTIES OUTPUT_NAME Avogadro)
TARGET_LINK_LIBRARIES(python-module stdc++ avogadro-lib)
