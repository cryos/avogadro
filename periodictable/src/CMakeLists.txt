include(${QT_USE_FILE})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

# Ensure the Avogadro include directory is always first
get_directory_property(tmp_include_dirs INCLUDE_DIRECTORIES)
set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES
  "${libavogadro_BINARY_DIR}/include;${tmp_include_dirs}")
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${EIGEN2_INCLUDE_DIR}
  ${OPENBABEL2_INCLUDE_DIR}
)

# I think this is necessary now in order to tell the linker where openbabel is
link_directories(${OPENBABEL2_LIBRARY_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/lib)

file(GLOB periodictable_SRCS
  elementdetail_p.cpp
  elementitem_p.cpp
  elementtranslator.cpp
  periodictablescene_p.cpp
  periodictableview.cpp
)

# Only headers with exported symbols should actually be installed
set(periodictable_HDRS
  elementtranslator.h
  periodictableview.h
)

set(PERIODICTABLE_LINK_LIBRARIES ${OPENBABEL2_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

qt4_automoc(${periodictable_SRCS})

# Set up the include directory and symlink/copy all headers to it.
# Note: Windows lacks symlinks and so the files must be copied.
make_directory("${libavogadro_BINARY_DIR}/include/qperiodictable")
if(UNIX)
  set(header_cmd "create_symlink")
else(UNIX)
  set(header_cmd "copy_if_different")
endif(UNIX)
foreach(headerFile ${periodictable_HDRS} global.h)
  set(from "${CMAKE_CURRENT_SOURCE_DIR}/${headerFile}")
  set(to   "${libavogadro_BINARY_DIR}/include/qperiodictable/${headerFile}")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E ${header_cmd} "${from}" "${to}")
endforeach(headerFile ${periodictable_HDRS})

add_library(periodictable SHARED
  ${periodictable_SRCS} )
set_target_properties(periodictable PROPERTIES
  LINK_INTERFACE_LIBRARIES ""
  VERSION ${Avogadro_VERSION_FULL}
  SOVERSION 1
  OUTPUT_NAME QPeriodicTable
  LABELS periodictable)
set_source_files_properties(${periodictable_SRCS}
  PROPERTIES LABELS periodictable)
target_link_libraries(periodictable ${PERIODICTABLE_LINK_LIBRARIES})

# Standalone periodic table
qt4_automoc(qperiodictable.cpp)
add_executable(qperiodictable qperiodictable.cpp)
target_link_libraries(qperiodictable periodictable ${OPENBABEL2_LIBRARIES}
  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

install(TARGETS periodictable DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS qperiodictable DESTINATION ${BIN_INSTALL_DIR})
install(FILES ${periodictable_HDRS}
  DESTINATION "${INCLUDE_INSTALL_DIR}/avogadro")
install(FILES ${periodictable_HDRS} global.h
  DESTINATION "${INCLUDE_INSTALL_DIR}/qperiodictable")
  
