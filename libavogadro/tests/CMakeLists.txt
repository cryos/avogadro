set(QT_USE_QTOPENGL true)

include(${QT_USE_FILE})

include_directories(
  ${libavogadro_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${EIGEN2_INCLUDE_DIR}
  ${OPENBABEL2_INCLUDE_DIR}
  ${BOOST_PYTHON_INCLUDES}
  ${PYTHON_INCLUDE_PATH}
)

link_directories(
  ${libavogadro_BINARY_DIR}/src
  ${OPENBABEL2_LIBRARY_DIRS}
)

# Disabled hydrogenscommand test for now - needs some work. Moc issues here.
# The commands should not just be included, they either need linking to
# or building. As plugin code is not part of the library it may require a
# different testing strategy.
set(tests
  drawcommand
#  hydrogenscommand
  molecule
  neighborlist
)

foreach (test ${tests})
  message(STATUS "Test:  ${test}")
  set(test_SRCS ${test}test.cpp)
  set(test_MOC_CPPS ${test}test.cpp)
  QT4_WRAP_CPP(test_MOC_SRCS ${test_MOC_CPPS})
  ADD_CUSTOM_TARGET(${test}testmoc ALL DEPENDS ${test_MOC_SRCS})
  add_executable(${test}test ${test_SRCS})
  add_dependencies(${test}test ${test}testmoc)
  target_link_libraries(${test}test
    ${OPENBABEL2_LIBRARIES}
    ${QT_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    avogadro-lib)
  add_test(${test}Test ${CMAKE_BINARY_DIR}/bin/${test}test)
endforeach (test ${tests})
