include(${QT_USE_FILE})

include_directories(
  ${libavogadro_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${OPENBABEL2_INCLUDE_DIR}
  ${BOOST_PYTHON_INCLUDES}
  ${PYTHON_INCLUDE_PATH}
)

link_directories(
  ${libavogadro_BINARY_DIR}/src
  ${OPENBABEL2_LIBRARY_DIRS}
)

# This test puts the Molecule class through its paces
set(moleculetest_SRCS moleculetest.cpp)
set(moleculetest_MOC_CPPS moleculetest.cpp)
QT4_WRAP_CPP(moleculetest_MOC_SRCS ${moleculetest_MOC_CPPS})
ADD_CUSTOM_TARGET(moleculetestmoc ALL DEPENDS
  ${moleculetest_MOC_SRCS})
add_executable(moleculetest
  ${moleculetest_SRCS})
add_dependencies(moleculetest moleculetestmoc)
target_link_libraries(moleculetest
  ${OPENBABEL2_LIBRARIES}
  ${QT_LIBRARIES}
  ${QT_QTTEST_LIBRARY}
  avogadro-lib)
#add_test(MoleculeTest ${EXECUTABLE_OUTPUT_PATH}/moleculetest)
add_test(MoleculeTest moleculetest)
