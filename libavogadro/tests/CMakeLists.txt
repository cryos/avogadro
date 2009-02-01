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
#set(moleculetest_SRCS moleculetest.cpp)
#set(moleculetest_MOC_CPPS moleculetest.cpp)
#QT4_WRAP_CPP(moleculetest_MOC_SRCS ${moleculetest_MOC_CPPS})
#ADD_CUSTOM_TARGET(moleculetestmoc ALL DEPENDS
#  ${moleculetest_MOC_SRCS})
#add_executable(moleculetest
#  ${moleculetest_SRCS})
#add_dependencies(moleculetest moleculetestmoc)
#target_link_libraries(moleculetest
#  ${OPENBABEL2_LIBRARIES}
#  ${QT_LIBRARIES}
#  ${QT_QTTEST_LIBRARY}
#  avogadro-lib)

set(tests
    molecule
    neighborlist
   )

foreach (test ${tests})
message(STATUS "Test:  ${test}")
set(test_SRCS ${test}test.cpp)
set(test_MOC_CPPS ${test}test.cpp)
# This test puts the Molecule class through its paces
QT4_WRAP_CPP(test_MOC_SRCS ${test_MOC_CPPS})
ADD_CUSTOM_TARGET(${test}testmoc ALL DEPENDS ${test_MOC_SRCS})
add_executable(${test}test ${test_SRCS})
add_dependencies(${test}test ${test}testmoc)
target_link_libraries(${test}test
  ${OPENBABEL2_LIBRARIES}
  ${QT_LIBRARIES}
  ${QT_QTTEST_LIBRARY}
  avogadro-lib)

endforeach (test ${tests})

  
  
add_test(MoleculeTest moleculetest)
add_test(NeighborListTest neighborlisttest)
